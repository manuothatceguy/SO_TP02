// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#ifdef BUDDY
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <textModule.h>
#include <debug.h>

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
static inline void initMemoryBlock(BuddyBlock * block, uint8_t level);
static inline uint64_t calculateOffset(MemoryManagerADT memoryManager, BuddyBlock * block);
static inline uint8_t memoryToLevel(uint64_t memoryToAllocate);
static BuddyBlock* getBuddy(MemoryManagerADT memoryManager, BuddyBlock* block);
static void mergeBlocks(MemoryManagerADT memoryManager, BuddyBlock* block);
static BuddyBlock* splitBlockToLevel(MemoryManagerADT memoryManager, BuddyBlock* block, uint8_t currentLevel, uint8_t targetLevel);

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
    DEBUG_PRINT("[createMemoryManager] Bloque raíz en nivel: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(maxLevel, DEBUG_COLOR);
    DEBUG_PRINT(" dirección: ", DEBUG_COLOR);
    DEBUG_PRINT_INT((uint64_t)rootBlock, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);
    DEBUG_PRINT("[createMemoryManager] Estado inicial de blocks: ", DEBUG_COLOR);
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            DEBUG_PRINT("[", DEBUG_COLOR);
            DEBUG_PRINT_INT(i, DEBUG_COLOR);
            DEBUG_PRINT(":L]", DEBUG_COLOR);
        }
    }
    DEBUG_PRINT("\n", DEBUG_COLOR);
}

void *allocMemory(const size_t memoryToAllocate) {
    if(memoryManager == NULL || memoryManager->status != BUDDY_OK) {
        DEBUG_PRINT("No se ha creado un memory manager o hay un error\n", DEBUG_COLOR);
        return NULL;
    }

    // Sumar el tamaño del header
    uint64_t size = memoryToAllocate + sizeof(BuddyBlock);
    DEBUG_PRINT("[allocMemory] Tamaño solicitado: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(memoryToAllocate, DEBUG_COLOR);
    DEBUG_PRINT(" + header = ", DEBUG_COLOR);
    DEBUG_PRINT_INT(size, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);

    if (size < BUDDY_MIN_BLOCK_SIZE) {
        DEBUG_PRINT("El tamaño mínimo de bloque es 8 bytes\n", DEBUG_COLOR);
        size = BUDDY_MIN_BLOCK_SIZE;
    }

    // Ajustar a la siguiente potencia de 2
    if ((size & (size - 1)) != 0) {
        DEBUG_PRINT("El tamaño debe ser una potencia de 2\n", DEBUG_COLOR);
        size = 1ULL << (log2(size) + 1);
    }
    DEBUG_PRINT("[allocMemory] Tamaño ajustado: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(size, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);

    uint8_t level = memoryToLevel(size);
    DEBUG_PRINT("[allocMemory] Nivel calculado: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(level, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);

    // Mostrar el estado del array blocks
    DEBUG_PRINT("[allocMemory] Estado de blocks: ", DEBUG_COLOR);
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
            DEBUG_PRINT("[", DEBUG_COLOR);
            DEBUG_PRINT_INT(i, DEBUG_COLOR);
            DEBUG_PRINT(":L]", DEBUG_COLOR);
        }
    }
    DEBUG_PRINT("\n", DEBUG_COLOR);

    if (level >= BLOCKS) {
        memoryManager->status = BUDDY_ERROR;
        DEBUG_PRINT("Tamaño de bloque demasiado grande\n", DEBUG_COLOR);
        return NULL; // Tamaño demasiado grande
    }
    
    // Buscar un bloque libre del tamaño adecuado
    BuddyBlock* block = findFreeBlock(memoryManager, level);
    if (block == NULL) {
        DEBUG_PRINT("No hay bloques libres disponibles\n", DEBUG_COLOR);
        return NULL; // No hay memoria disponible
    }
    
    // Marcar el bloque como asignado
    block->blockState = ALLOCATED;
    DEBUG_PRINT("Bloque asignado\n", DEBUG_COLOR);
    
    // Calcular y retornar la dirección de memoria después del header
    memoryManager->usedMemory += (1ULL << level);
    DEBUG_PRINT("[allocMemory] Memoria usada: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(memoryManager->usedMemory, DEBUG_COLOR);
    DEBUG_PRINT(" / ", DEBUG_COLOR);
    DEBUG_PRINT_INT(memoryManager->memorySize, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);
    return (void*)((char*)block + sizeof(BuddyBlock));
}

static BuddyBlock * findFreeBlock(MemoryManagerADT memoryManager, uint8_t level) {
    DEBUG_PRINT("[findFreeBlock] Buscando nivel: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(level, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);
    
    // Buscar en el nivel actual
    if (memoryManager->blocks[level] != NULL) {
        DEBUG_PRINT("[findFreeBlock] Bloque encontrado en el nivel actual\n", DEBUG_COLOR);
        BuddyBlock* block = memoryManager->blocks[level];
        
        // Verificar que el bloque esté realmente libre
        if (block->blockState != FREE) {
            DEBUG_PRINT("[findFreeBlock] El bloque no está libre\n", DEBUG_COLOR);
            return NULL;
        }
        
        // Quitar de la lista de libres
        memoryManager->blocks[level] = block->next;
        if (block->next) {
            DEBUG_PRINT("[findFreeBlock] Bloque siguiente encontrado\n", DEBUG_COLOR);
            block->next->prev = NULL;
        }
        
        block->next = NULL;
        block->prev = NULL;
        return block;
    }
    
    // Si no hay bloques en este nivel, intentar encontrar un bloque en un nivel superior
    for (uint8_t i = level + 1; i < BLOCKS; i++) {
        BuddyBlock* block = memoryManager->blocks[i];
        if (block != NULL) {
            DEBUG_PRINT("[findFreeBlock] Dividiendo bloque de nivel superior: ", DEBUG_COLOR);
            DEBUG_PRINT_INT(i, DEBUG_COLOR);
            DEBUG_PRINT("\n", DEBUG_COLOR);
            
            // Remover el bloque del nivel superior
            memoryManager->blocks[i] = block->next;
            if (block->next) {
                block->next->prev = NULL;
            }
            block->next = NULL;
            block->prev = NULL;
            
            // Asegurarse de que cada nivel de split añada correctamente ambos bloques
            BuddyBlock* result = splitBlockToLevel(memoryManager, block, i, level);
            if (result != NULL) {
                return result;
            }
            // Si el split falló, devolver el bloque original a la lista de libres
            block->next = memoryManager->blocks[i];
            if (memoryManager->blocks[i]) {
                memoryManager->blocks[i]->prev = block;
            }
            memoryManager->blocks[i] = block;
        }
    }
    
    DEBUG_PRINT("[findFreeBlock] No hay bloques libres disponibles\n", DEBUG_COLOR);
    return NULL; // No hay bloques disponibles
}

static BuddyBlock* splitBlockToLevel(MemoryManagerADT memoryManager, BuddyBlock* block, uint8_t currentLevel, uint8_t targetLevel) {
    DEBUG_PRINT("[splitBlockToLevel] Dividiendo bloque de nivel ", DEBUG_COLOR);
    DEBUG_PRINT_INT(currentLevel, DEBUG_COLOR);
    DEBUG_PRINT(" a nivel ", DEBUG_COLOR);
    DEBUG_PRINT_INT(targetLevel, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);
    
    if (currentLevel <= targetLevel) {
        DEBUG_PRINT("[splitBlockToLevel] Ya estamos en el nivel objetivo o inferior\n", DEBUG_COLOR);
        return (currentLevel == targetLevel) ? block : NULL;
    }
    
    // Mark block as split
    block->blockState = SPLIT;
    
    // Calculate new level and child size
    uint8_t newLevel = currentLevel - 1;
    uint64_t childSize = (1ULL << newLevel);
    
    // Calculate positions for children
    BuddyBlock* leftChild = block;
    BuddyBlock* rightChild = (BuddyBlock*)((char*)block + childSize);
    
    // Initialize children
    initMemoryBlock(leftChild, newLevel);
    initMemoryBlock(rightChild, newLevel);
    
    // Add right child to free list
    rightChild->next = memoryManager->blocks[newLevel];
    if (memoryManager->blocks[newLevel]) {
        memoryManager->blocks[newLevel]->prev = rightChild;
    }
    memoryManager->blocks[newLevel] = rightChild;
    
    // If we reached target level, return left child
    if (newLevel == targetLevel) {
        return leftChild;
    }
    
    // Otherwise, continue splitting left child
    return splitBlockToLevel(memoryManager, leftChild, newLevel, targetLevel);
}

static inline uint64_t calculateOffset(MemoryManagerADT memoryManager, BuddyBlock * block) {
    // Calcular el offset del bloque en la memoria gestionada
    return (uint64_t)((char*)block - (char*)memoryManager->memoryForMemoryManager);
}

void *freeMemory(void *const restrict address) {
    if (address == NULL || memoryManager == NULL || memoryManager->status != BUDDY_OK) {
        DEBUG_PRINT("Dirección inválida o memory manager no creado\n", DEBUG_COLOR);
        return NULL;
    }

    // Obtener el bloque real restando el header
    BuddyBlock* block = (BuddyBlock*)((char*)address - sizeof(BuddyBlock));

    // Validar el bloque
    if (block->blockState != ALLOCATED) {
        DEBUG_PRINT("Bloque no encontrado o no asignado\n", DEBUG_COLOR);
        return NULL; // Dirección inválida o bloque no asignado
    }

    DEBUG_PRINT("[freeMemory] Liberando bloque de nivel: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(block->level, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);

    // Liberar el bloque
    block->blockState = FREE;

    // Añadir el bloque a la lista de libres de su nivel
    block->next = memoryManager->blocks[block->level];
    if (memoryManager->blocks[block->level]) {
        DEBUG_PRINT("Bloque añadido a la lista de libres\n", DEBUG_COLOR);
        memoryManager->blocks[block->level]->prev = block;
    }
    memoryManager->blocks[block->level] = block;

    // Actualizar la memoria usada
    memoryManager->usedMemory -= (1ULL << block->level);
    DEBUG_PRINT("[freeMemory] Memoria usada: ", DEBUG_COLOR);
    DEBUG_PRINT_INT(memoryManager->usedMemory, DEBUG_COLOR);
    DEBUG_PRINT(" / ", DEBUG_COLOR);
    DEBUG_PRINT_INT(memoryManager->memorySize, DEBUG_COLOR);
    DEBUG_PRINT("\n", DEBUG_COLOR);

    // Intentar fusionar con el buddy
    mergeBlocks(memoryManager, block);

    return NULL;
}

static BuddyBlock* getBuddy(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS) {
        DEBUG_PRINT("Bloque inválido o nivel fuera de rango\n", DEBUG_COLOR);
        return NULL;
    }
    
    // Calculate buddy address using XOR
    uint64_t blockAddr = (uint64_t)block;
    uint64_t buddyAddr = blockAddr ^ (1ULL << block->level);
    
    // Verify buddy is within managed memory
    if (buddyAddr < (uint64_t)memoryManager->managedMemory || 
        buddyAddr >= (uint64_t)memoryManager->managedMemory + memoryManager->memorySize) {
        DEBUG_PRINT("Buddy fuera de rango\n", DEBUG_COLOR);
        return NULL;
    }
    
    return (BuddyBlock*)buddyAddr;
}

static void mergeBlocks(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS || block->blockState != FREE) {
        DEBUG_PRINT("Bloque inválido o no libre\n", DEBUG_COLOR);
        return;
    }
    
    // Encontrar el buddy
    BuddyBlock* buddy = getBuddy(memoryManager, block);
    
    // Si el buddy está libre, fusionarlos
    if (buddy != NULL && buddy->blockState == FREE && buddy->level == block->level) {
        // Quitar ambos bloques de la lista de libres
        if (buddy->prev) {
            DEBUG_PRINT("Bloque buddy encontrado\n", DEBUG_COLOR);
            buddy->prev->next = buddy->next;
        } else {
            DEBUG_PRINT("Bloque buddy es el primero\n", DEBUG_COLOR);
            memoryManager->blocks[buddy->level] = buddy->next;
        }
        if (buddy->next) {
            DEBUG_PRINT("Bloque buddy siguiente encontrado\n", DEBUG_COLOR);
            buddy->next->prev = buddy->prev;
        }
        
        if (block->prev) {
            DEBUG_PRINT("Bloque anterior encontrado\n", DEBUG_COLOR);
            block->prev->next = block->next;
        } else {
            DEBUG_PRINT("Bloque anterior es el primero\n", DEBUG_COLOR);
            memoryManager->blocks[block->level] = block->next;
        }
        if (block->next) {
            DEBUG_PRINT("Bloque siguiente encontrado\n", DEBUG_COLOR);
            block->next->prev = block->prev;
        }
        
        // Determinar cuál es el bloque padre (el de menor dirección)
        BuddyBlock* parent = (block < buddy) ? block : buddy;
        
        // Inicializar el bloque padre
        initMemoryBlock(parent, block->level + 1);
        
        // Añadir el padre a la lista de libres
        parent->next = memoryManager->blocks[parent->level];
        if (memoryManager->blocks[parent->level]) {
            DEBUG_PRINT("Bloque padre añadido a la lista de libres\n", DEBUG_COLOR);
            memoryManager->blocks[parent->level]->prev = parent;
        }
        DEBUG_PRINT("Bloque padre añadido a la lista de libres\n", DEBUG_COLOR);
        memoryManager->blocks[parent->level] = parent;
        
        // Intentar fusionar recursivamente
        mergeBlocks(memoryManager, parent);
    }
}

static inline void initMemoryBlock(BuddyBlock * block, uint8_t level) {
    DEBUG_PRINT("Inicializando bloque de memoria\n", DEBUG_COLOR);
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

void getMemoryInfo(memInfo *info) {
    if (info == NULL) {
        return;
    }
    info->total = memoryManager->memorySize;
    info->used = memoryManager->usedMemory;
    info->free = info->total - info->used;
}

#endif