#ifdef BUDDY
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <textModule.h>

#define BUDDY_MIN_BLOCK_SIZE 8 // 2^3 = 8 bytes
#define MAX_LEVEL 32  // Maximum level for buddy system
#define BLOCKS (MAX_LEVEL + 1)  // One block per level

enum state {FREE, SPLIT, ALLOCATED};
enum buddyStatus {BUDDY_OK, BUDDY_ERROR};

typedef struct BuddyBlock {
    uint8_t level;
    enum state blockState;
    struct BuddyBlock * next;
    struct BuddyBlock * prev;
} BuddyBlock;

typedef struct MemoryManager {
    void *memoryForMemoryManager;
    void *managedMemory;
    uint64_t memorySize;
    uint64_t usedMemory;
    enum buddyStatus status;
    BuddyBlock * blocks[BLOCKS];
} MemoryManager;

typedef MemoryManager* MemoryManagerADT;

// Function declarations
static BuddyBlock * findFreeBlock(MemoryManagerADT memoryManager, uint8_t level);
static BuddyBlock * splitBlock(MemoryManagerADT memoryManager, uint8_t sourceLevel, uint8_t targetLevel);
static inline void initMemoryBlock(BuddyBlock * block, uint8_t level);
static inline uint64_t calculateOffset(MemoryManagerADT memoryManager, BuddyBlock * block);
static inline uint8_t memoryToLevel(uint64_t memoryToAllocate);
static BuddyBlock* getBuddy(MemoryManagerADT memoryManager, BuddyBlock* block);
static void mergeBlocks(MemoryManagerADT memoryManager, BuddyBlock* block);

static MemoryManager * memoryManager = NULL;

uint64_t log2(uint64_t x) {
    uint8_t result = 0;
    while (x /= 2) {
        result++;
    }
    return result;
}

void createMemoryManager() {
    memoryManager = (MemoryManager *) MEMORY_MANAGER_ADDRESS;
    memoryManager->memoryForMemoryManager = (void *) MEMORY_MANAGER_ADDRESS;
    memoryManager->managedMemory = (void *) HEAP_START_ADDRESS;
    memoryManager->memorySize = HEAP_SIZE;
    memoryManager->usedMemory = 0;
    memoryManager->status = BUDDY_OK;  // Initialize status

    // Verify memory alignment
    if ((uint64_t)memoryManager->managedMemory % BUDDY_MIN_BLOCK_SIZE != 0) {
        memoryManager->status = BUDDY_ERROR;
        return;
    }

    // init
    for (uint64_t i = 1; i < BLOCKS; i++) {
        memoryManager->blocks[i] = NULL;
    }
    
    // Calcular la mayor potencia de 2 menor o igual a memorySize
    uint64_t rootBlockSize = 1ULL << log2(memoryManager->memorySize);
    uint8_t maxLevel = log2(rootBlockSize);

    // El bloque raíz debe empezar en managedMemory
    BuddyBlock* rootBlock = (BuddyBlock*)memoryManager->managedMemory;
    initMemoryBlock(rootBlock, maxLevel);
    memoryManager->blocks[maxLevel] = rootBlock;

    // Prints de depuración
    printStr("[createMemoryManager] Bloque raíz en nivel: ", 0x00FFFFFF);
    printInt(maxLevel, 0x00FFFFFF);
    printStr(" dirección: ", 0x00FFFFFF);
    printInt((uint64_t)rootBlock, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    printStr("[createMemoryManager] Estado inicial de blocks: ", 0x00FFFFFF);
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            printStr("[", 0x00FFFFFF);
            printInt(i, 0x00FFFFFF);
            printStr(":L]", 0x00FFFFFF);
        }
    }
    printStr("\n", 0x00FFFFFF);
}

void *allocMemory(const size_t memoryToAllocate) {
    if(memoryManager == NULL || memoryManager->status != BUDDY_OK) {
        printStr("No se ha creado un memory manager o hay un error\n", 0x00FFFFFF);
        return NULL;
    }

    // Sumar el tamaño del header
    uint64_t size = memoryToAllocate + sizeof(BuddyBlock);
    printStr("[allocMemory] Tamaño solicitado: ", 0x00FFFFFF);
    printInt(memoryToAllocate, 0x00FFFFFF);
    printStr(" + header = ", 0x00FFFFFF);
    printInt(size, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);

    if (size < BUDDY_MIN_BLOCK_SIZE) {
        printStr("El tamaño mínimo de bloque es 8 bytes\n", 0x00FFFFFF);
        size = BUDDY_MIN_BLOCK_SIZE;
    }

    // Ajustar a la siguiente potencia de 2
    if ((size & (size - 1)) != 0) {
        printStr("El tamaño debe ser una potencia de 2\n", 0x00FFFFFF);
        size = 1ULL << (log2(size) + 1);
    }
    printStr("[allocMemory] Tamaño ajustado: ", 0x00FFFFFF);
    printInt(size, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);

    uint8_t level = memoryToLevel(size);
    printStr("[allocMemory] Nivel calculado: ", 0x00FFFFFF);
    printInt(level, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);

    // Mostrar el estado del array blocks
    printStr("[allocMemory] Estado de blocks: ", 0x00FFFFFF);
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            printStr("[", 0x00FFFFFF);
            printInt(i, 0x00FFFFFF);
            printStr(":L]", 0x00FFFFFF);
        }
    }
    printStr("\n", 0x00FFFFFF);

    if (level >= BLOCKS) {
        memoryManager->status = BUDDY_ERROR;
        printStr("Tamaño de bloque demasiado grande\n", 0x00FFFFFF);
        return NULL; // Tamaño demasiado grande
    }
    
    // Buscar un bloque libre del tamaño adecuado
    BuddyBlock* block = findFreeBlock(memoryManager, level);
    if (block == NULL) {
        printStr("No hay bloques libres disponibles\n", 0x00FFFFFF);
        return NULL; // No hay memoria disponible
    }
    
    // Marcar el bloque como asignado
    block->blockState = ALLOCATED;
    printStr("Bloque asignado\n", 0x00FFFFFF);
    
    // Calcular y retornar la dirección de memoria después del header
    memoryManager->usedMemory += (1ULL << block->level);
    return (void*)((char*)block + sizeof(BuddyBlock));
}

static BuddyBlock * findFreeBlock(MemoryManagerADT memoryManager, uint8_t level) {
    printStr("[findFreeBlock] Buscando nivel: ", 0x00FFFFFF);
    printInt(level, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    // Buscar en el nivel actual
    if (memoryManager->blocks[level] != NULL) {
        printStr("[findFreeBlock] Bloque encontrado en el nivel actual\n", 0x00FFFFFF);
        BuddyBlock* block = memoryManager->blocks[level];
        
        // Verificar que el bloque esté realmente libre
        if (block->blockState != FREE) {
            printStr("[findFreeBlock] El bloque no está libre\n", 0x00FFFFFF);
            return NULL;
        }
        
        // Quitar de la lista de libres
        memoryManager->blocks[level] = block->next;
        if (block->next) {
            printStr("[findFreeBlock] Bloque siguiente encontrado\n", 0x00FFFFFF);
            block->next->prev = NULL;
        }
        
        block->next = NULL;
        block->prev = NULL;
        return block;
    }
    
    // Si no hay bloques en este nivel, dividir uno de nivel superior
    for (uint8_t i = level + 1; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            printStr("[findFreeBlock] Dividiendo bloque de nivel superior: ", 0x00FFFFFF);
            printInt(i, 0x00FFFFFF);
            printStr("\n", 0x00FFFFFF);
            // Dividir bloque y retornar uno de los hijos
            return splitBlock(memoryManager, i, level);
        }
    }
    printStr("[findFreeBlock] No hay bloques libres disponibles\n", 0x00FFFFFF);
    return NULL; // No hay bloques disponibles
}

static BuddyBlock * splitBlock(MemoryManagerADT memoryManager, uint8_t sourceLevel, uint8_t targetLevel) {
    printStr("[splitBlock] INICIO: de nivel ", 0x00FFFFFF);
    printInt(sourceLevel, 0x00FFFFFF);
    printStr(" a nivel ", 0x00FFFFFF);
    printInt(targetLevel, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    if (sourceLevel <= targetLevel) {
        printStr("[splitBlock] No se puede dividir a un nivel mayor o igual\n", 0x00FFFFFF);
        return NULL; // No se puede dividir a un nivel mayor o igual
    }

    // Obtener un bloque libre del nivel sourceLevel
    BuddyBlock* block = memoryManager->blocks[sourceLevel];
    if (block == NULL) {
        printStr("[splitBlock] No se pudo encontrar un bloque libre\n", 0x00FFFFFF);
        return NULL;
    }
    // Eliminar el bloque de la lista de libres antes de dividir
    memoryManager->blocks[sourceLevel] = block->next;
    if (block->next) {
        block->next->prev = NULL;
    }
    block->next = NULL;
    block->prev = NULL;

    printStr("[splitBlock] Dividiendo bloque en dirección: ", 0x00FFFFFF);
    printInt((uint64_t)block, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);

    block->blockState = SPLIT;

    // Calcular el tamaño del bloque hijo
    uint8_t newLevel = sourceLevel - 1;
    uint64_t childSize = (1ULL << newLevel);

    // Calcular las posiciones de los hijos
    BuddyBlock* leftChild = block;
    BuddyBlock* rightChild = (BuddyBlock*)((char*)block + childSize);

    // Inicializar los hijos
    initMemoryBlock(leftChild, newLevel);
    initMemoryBlock(rightChild, newLevel);

    // Añadir el bloque derecho a la lista de libres de su nivel
    rightChild->next = memoryManager->blocks[newLevel];
    if (memoryManager->blocks[newLevel]) {
        printStr("[splitBlock] Bloque derecho añadido a la lista de libres\n", 0x00FFFFFF);
        memoryManager->blocks[newLevel]->prev = rightChild;
    }
    memoryManager->blocks[newLevel] = rightChild;
    printStr("[splitBlock] Estado de blocks tras split: ", 0x00FFFFFF);
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            printStr("[", 0x00FFFFFF);
            printInt(i, 0x00FFFFFF);
            printStr(":L]", 0x00FFFFFF);
        }
    }
    printStr("\n", 0x00FFFFFF);

    // Si hemos llegado al nivel objetivo, devolver el bloque izquierdo
    if (newLevel == targetLevel) {
        printStr("[splitBlock] Bloque izquierdo devuelto\n", 0x00FFFFFF);
        return leftChild;
    }

    // De lo contrario, seguir dividiendo el bloque izquierdo
    return splitBlock(memoryManager, newLevel, targetLevel);
}

static inline uint64_t calculateOffset(MemoryManagerADT memoryManager, BuddyBlock * block) {
    // Calcular el offset del bloque en la memoria gestionada
    return (uint64_t)((char*)block - (char*)memoryManager->memoryForMemoryManager);
}

void *freeMemory(void *const restrict address) {
    if (address == NULL || memoryManager == NULL || memoryManager->status != BUDDY_OK) {
        printStr("Dirección inválida o memory manager no creado\n", 0x00FFFFFF);
        return NULL;
    }

    // Obtener el bloque real restando el header
    BuddyBlock* block = (BuddyBlock*)((char*)address - sizeof(BuddyBlock));

    // Validar el bloque
    if (block->blockState != ALLOCATED) {
        memoryManager->status = BUDDY_ERROR;
        printStr("Bloque no encontrado o no asignado\n", 0x00FFFFFF);
        return NULL; // Dirección inválida o bloque no asignado
    }

    // Liberar el bloque
    block->blockState = FREE;

    // Añadir el bloque a la lista de libres de su nivel
    block->next = memoryManager->blocks[block->level];
    if (memoryManager->blocks[block->level]) {
        printStr("Bloque añadido a la lista de libres\n", 0x00FFFFFF);
        memoryManager->blocks[block->level]->prev = block;
    }
    memoryManager->blocks[block->level] = block;

    // Actualizar la memoria usada
    memoryManager->usedMemory -= (1ULL << block->level);

    // Intentar fusionar con el buddy
    mergeBlocks(memoryManager, block);

    return NULL;
}

static BuddyBlock* getBuddy(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS) {
        printStr("Bloque inválido o nivel fuera de rango\n", 0x00FFFFFF);
        return NULL;
    }
    
    // Calculate buddy address using XOR
    uint64_t blockAddr = (uint64_t)block;
    uint64_t buddyAddr = blockAddr ^ (1ULL << block->level);
    
    // Verify buddy is within managed memory
    if (buddyAddr < (uint64_t)memoryManager->managedMemory || 
        buddyAddr >= (uint64_t)memoryManager->managedMemory + memoryManager->memorySize) {
        printStr("Buddy fuera de rango\n", 0x00FFFFFF);
        return NULL;
    }
    
    return (BuddyBlock*)buddyAddr;
}

static void mergeBlocks(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS || block->blockState != FREE) {
        printStr("Bloque inválido o no libre\n", 0x00FFFFFF);
        return;
    }
    
    // Encontrar el buddy
    BuddyBlock* buddy = getBuddy(memoryManager, block);
    
    // Si el buddy está libre, fusionarlos
    if (buddy != NULL && buddy->blockState == FREE && buddy->level == block->level) {
        // Quitar ambos bloques de la lista de libres
        if (buddy->prev) {
            printStr("Bloque buddy encontrado\n", 0x00FFFFFF);
            buddy->prev->next = buddy->next;
        } else {
            printStr("Bloque buddy es el primero\n", 0x00FFFFFF);
            memoryManager->blocks[buddy->level] = buddy->next;
        }
        if (buddy->next) {
            printStr("Bloque buddy siguiente encontrado\n", 0x00FFFFFF);
            buddy->next->prev = buddy->prev;
        }
        
        if (block->prev) {
            printStr("Bloque anterior encontrado\n", 0x00FFFFFF);
            block->prev->next = block->next;
        } else {
            printStr("Bloque anterior es el primero\n", 0x00FFFFFF);
            memoryManager->blocks[block->level] = block->next;
        }
        if (block->next) {
            printStr("Bloque siguiente encontrado\n", 0x00FFFFFF);
            block->next->prev = block->prev;
        }
        
        // Determinar cuál es el bloque padre (el de menor dirección)
        BuddyBlock* parent = (block < buddy) ? block : buddy;
        
        // Inicializar el bloque padre
        initMemoryBlock(parent, block->level + 1);
        
        // Añadir el padre a la lista de libres
        parent->next = memoryManager->blocks[parent->level];
        if (memoryManager->blocks[parent->level]) {
            printStr("Bloque padre añadido a la lista de libres\n", 0x00FFFFFF);
            memoryManager->blocks[parent->level]->prev = parent;
        }
        printStr("Bloque padre añadido a la lista de libres\n", 0x00FFFFFF);
        memoryManager->blocks[parent->level] = parent;
        
        // Intentar fusionar recursivamente
        mergeBlocks(memoryManager, parent);
    }
}

static inline void initMemoryBlock(BuddyBlock * block, uint8_t level) {
    printStr("Inicializando bloque de memoria\n", 0x00FFFFFF);
    block->level = level;
    block->blockState = FREE;
    block->next = NULL;
    block->prev = NULL;
}

static inline uint8_t memoryToLevel(uint64_t memoryToAllocate) {
    uint8_t level = log2(memoryToAllocate);
    if ((1ULL << level) < memoryToAllocate) {
        level++;
    }
    return level;
}

#endif