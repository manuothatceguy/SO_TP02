#ifdef buddy
#include <memoryManager.h>
#include <defs.h>

#define BUDDY_MIN_BLOCK_SIZE 8 // 2^3 = 8 bytes

enum state {FREE, SPLIT, ALLOCATED};

static BuddyBlock * findFreeBlock(MemoryManagerADT memoryManager, uint8_t level);
static BuddyBlock * splitBlock(MemoryManagerADT memoryManager, uint8_t sourceLevel, uint8_t targetLevel);
static inline void initMemoryBlock(BuddyBlock * block, uint8_t level);
static inline uint64_t calculateOffset(MemoryManagerADT memoryManager, BuddyBlock * block);
static inline uint8_t memoryToLevel(uint64_t memoryToAllocate);

typedef struct BuddyBlock {
    uint8_t level;
    enum state blockState;
    struct BuddyBlock * next;
    struct BuddyBlock * prev;
} BuddyBlock;

#define BLOCKS (HEAP_SIZE / BUDDY_MIN_BLOCK_SIZE)

typedef struct MemoryManager {
    void *memoryForMemoryManager;
    void *managedMemory;
    uint64_t memorySize;
    uint64_t usedMemory;
    memStatus status;
    BuddyBlock * blocks[BLOCKS];
} MemoryManager;

uint64_t log2(uint64_t x) {
    uint8_t result = 0;
    while (x /= 2) {
        result++;
    }
    return result;
}

static inline void initMemoryBlock(BuddyBlock * block, uint8_t level);
static BuddyBlock * findFreeBlock(uint8_t level);
static BuddyBlock * splitBlock(uint8_t sourceLevel, uint8_t targetLevel);
static inline uint64_t calculateOffset(BuddyBlock * block);
static inline BuddyBlock * getBuddy(BuddyBlock * block);
static MemoryManager * memoryManager = NULL;

void createMemoryManager() {
    memoryManager = (MemoryManager *) MEMORY_MANAGER_ADDRESS;
    memoryManager->memoryForMemoryManager = (void *) MEMORY_MANAGER_ADDRESS;
    memoryManager->managedMemory = (void *) HEAP_START_ADDRESS;
    memoryManager->memorySize = HEAP_SIZE;
    memoryManager->usedMemory = 0;

    // init
    for (uint64_t i = 1; i < BLOCKS; i++) {
        memoryManager->blocks[i] = NULL;
    }
    
    // primer bloque (toda la memoria)
    uint8_t maxLevel = log2(memorySize);
    BuddyBlock* rootBlock = (BuddyBlock*)((char*)memoryManager + sizeof(MemoryManagerCDT));
    initMemoryBlock(rootBlock, maxLevel);
    memoryManager->blocks[maxLevel] = rootBlock;
}

void *allocMemory(const size_t memoryToAllocate) {
    if(memoryManager == NULL) {
        return NULL; // tenes que tener un memory manager creado, sino no anda.
    }
    uint64_t size = memoryToAllocate;
    if (size < BUDDY_MIN_BLOCK_SIZE) {
        size = BUDDY_MIN_BLOCK_SIZE;
    }

    uint8_t level = memoryToLevel(size);

    if (level >= BLOCKS) {
        return NULL; // Tamaño demasiado grande
    }
    
    // Buscar un bloque libre del tamaño adecuado
    BuddyBlock* block = findFreeBlock(memoryManager, level);
    if (block == NULL) {
        return NULL; // No hay memoria disponible
    }
    
    // Marcar el bloque como asignado
    block->blockState = ALLOCATED;
    
    // Calcular y retornar la dirección de memoria
    uint64_t offset = calculateOffset(memoryManager, block);
    memoryManager->usedMemory += (1ULL << block->level);
    return (void*)((char*)memoryManager->managedMemory + offset);
}

static BuddyBlock * findFreeBlock(uint8_t level) {
    // Buscar en el nivel actual
    if (memoryManager->blocks[level] != NULL) {
        BuddyBlock* block = memoryManager->blocks[level];
        
        // Quitar de la lista de libres
        memoryManager->blocks[level] = block->next;
        if (block->next) {
            block->next->prev = NULL;
        }
        
        block->next = NULL;
        block->prev = NULL;
        return block;
    }
    
    // Si no hay bloques en este nivel, dividir uno de nivel superior
    for (uint8_t i = level + 1; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            // Dividir bloque y retornar uno de los hijos
            return splitBlock(memoryManager, i, level);
        }
    }
    
    return NULL; // No hay bloques disponibles
}

static BuddyBlock * splitBlock(uint8_t sourceLevel, uint8_t targetLevel) {
    if (sourceLevel <= targetLevel) {
        return NULL; // No se puede dividir a un nivel mayor o igual
    }
    
    // Obtener un bloque libre del nivel sourceLevel
    BuddyBlock* block = findFreeBlock(memoryManager, sourceLevel);
    if (block == NULL) {
        return NULL;
    }
    
    // Dividir el bloque recursivamente hasta llegar al nivel objetivo
    block->blockState = SPLIT;
    
    // Crear dos bloques hijos (el segundo estará en memoria después del primero)
    uint8_t newLevel = sourceLevel - 1;
    BuddyBlock* leftChild = (BuddyBlock*)((char*)block + sizeof(BuddyBlock));
    BuddyBlock* rightChild = (BuddyBlock*)((char*)leftChild + (1ULL << newLevel));
    
    // Inicializar los hijos
    initMemoryBlock(leftChild, newLevel);
    initMemoryBlock(rightChild, newLevel);
    
    // Añadir el bloque derecho a la lista de libres de su nivel
    rightChild->next = memoryManager->blocks[newLevel];
    if (memoryManager->blocks[newLevel]) {
        memoryManager->blocks[newLevel]->prev = rightChild;
    }
    memoryManager->blocks[newLevel] = rightChild;
    
    // Si hemos llegado al nivel objetivo, devolver el bloque izquierdo
    if (newLevel == targetLevel) {
        return leftChild;
    }
    
    // De lo contrario, seguir dividiendo el bloque izquierdo
    return splitBlock(memoryManager, newLevel, targetLevel);
}

static inline uint64_t calculateOffset(BuddyBlock * block) {
    // Calcular el offset del bloque en la memoria gestionada
    return (uint64_t)((char*)block - (char*)memoryManager->memoryForMemoryManager);
}

void freeMemory(void *const restrict address) {
    if (address == NULL || memoryManager == NULL) {
        return;
    }
    
    // Calcular el offset de la dirección en la memoria gestionada
    uint64_t offset = (uint64_t)((char*)address - (char*)memoryManager->managedMemory);
    
    // Encontrar el bloque correspondiente
    BuddyBlock* block = NULL;
    
    // Buscar el bloque en la matriz de bloques
    for (uint8_t i = 0; i < BLOCKS; i++) {
        BuddyBlock* current = memoryManager->blocks[i];
        while (current != NULL) {
            if (calculateOffset(memoryManager, current) == offset) {
                block = current;
                break;
            }
            current = current->next;
        }
        if (block != NULL) {
            break;
        }
    }
    
    if (block == NULL || block->blockState != ALLOCATED) {
        return; // Dirección inválida o bloque no asignado
    }
    
    // Liberar el bloque
    block->blockState = FREE;
    
    // Añadir el bloque a la lista de libres de su nivel
    block->next = memoryManager->blocks[block->level];
    if (memoryManager->blocks[block->level]) {
        memoryManager->blocks[block->level]->prev = block;
    }
    memoryManager->blocks[block->level] = block;
    
    // Actualizar la memoria usada
    memoryManager->usedMemory -= (1ULL << block->level);

    // Intentar fusionar con el buddy
    mergeBlocks(memoryManager, block);
}

static void mergeBlocks(BuddyBlock* block) {
    if (block->level == 0 || block->blockState != FREE) {
        return;
    }
    
    // Encontrar el buddy
    BuddyBlock* buddy = getBuddy(memoryManager, block);
    
    // Si el buddy está libre, fusionarlos
    if (buddy != NULL && buddy->blockState == FREE) {
        // Quitar ambos bloques de la lista de libres
        if (buddy->prev) {
            buddy->prev->next = buddy->next;
        } else {
            memoryManager->blocks[buddy->level] = buddy->next;
        }
        if (buddy->next) {
            buddy->next->prev = buddy->prev;
        }
        
        if (block->prev) {
            block->prev->next = block->next;
        } else {
            memoryManager->blocks[block->level] = block->next;
        }
        if (block->next) {
            block->next->prev = block->prev;
        }
        
        // Determinar cuál es el bloque padre
        BuddyBlock* parent = (BuddyBlock*)((uintptr_t)block & ~(1ULL << block->level));
        
        // Inicializar el bloque padre
        initMemoryBlock(parent, block->level + 1);
        
        // Añadir el padre a la lista de libres
        parent->next = memoryManager->blocks[parent->level];
        if (memoryManager->blocks[parent->level]) {
            memoryManager->blocks[parent->level]->prev = parent;
        }
        memoryManager->blocks[parent->level] = parent;
        
        // Intentar fusionar recursivamente
        mergeBlocks(memoryManager, parent);
    }
}

static inline BuddyBlock* getBuddy(BuddyBlock* block) {
    // Calcular la dirección del buddy
    uint64_t blockAddr = (uint64_t)block;
    uint64_t buddyAddr = blockAddr ^ (1ULL << block->level);
    return (BuddyBlock*)buddyAddr;
}

static inline void initMemoryBlock(BuddyBlock * block, uint8_t level) {
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