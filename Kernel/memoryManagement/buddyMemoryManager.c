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

}

void *allocMemory(const size_t memoryToAllocate) {
    if(memoryManager == NULL || memoryManager->status != BUDDY_OK) {
        return NULL;
    }

    // Sumar el tamaño del header
    uint64_t size = memoryToAllocate + sizeof(BuddyBlock);

    if (size < BUDDY_MIN_BLOCK_SIZE) {
        size = BUDDY_MIN_BLOCK_SIZE;
    }

    // Ajustar a la siguiente potencia de 2
    if ((size & (size - 1)) != 0) {
        size = 1ULL << (log2(size) + 1);
    }

    uint8_t level = memoryToLevel(size);

    // Mostrar el estado del array blocks
    for (int i = 0; i < BLOCKS; i++) {
        if (memoryManager->blocks[i] != NULL) {
        }
    }

    if (level >= BLOCKS) {
        memoryManager->status = BUDDY_ERROR;
        return NULL; // Tamaño demasiado grande
    }
    
    // Buscar un bloque libre del tamaño adecuado
    BuddyBlock* block = findFreeBlock(memoryManager, level);
    if (block == NULL) {
        return NULL; // No hay memoria disponible
    }
    
    // Marcar el bloque como asignado
    block->blockState = ALLOCATED;
    
    // Calcular y retornar la dirección de memoria después del header
    memoryManager->usedMemory += (1ULL << level);
    return (void*)((char*)block + sizeof(BuddyBlock));
}

static BuddyBlock * findFreeBlock(MemoryManagerADT memoryManager, uint8_t level) {
    // Buscar en el nivel actual
    if (memoryManager->blocks[level] != NULL) {
        BuddyBlock* block = memoryManager->blocks[level];
        
        // Verificar que el bloque esté realmente libre
        if (block->blockState != FREE) {
            return NULL;
        }
        
        // Quitar de la lista de libres
        memoryManager->blocks[level] = block->next;
        if (block->next) {
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
    
    return NULL; // No hay bloques disponibles
}

static BuddyBlock* splitBlockToLevel(MemoryManagerADT memoryManager, BuddyBlock* block, uint8_t currentLevel, uint8_t targetLevel) {
    if (currentLevel <= targetLevel) {
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
        return NULL;
    }

    // Obtener el bloque real restando el header
    BuddyBlock* block = (BuddyBlock*)((char*)address - sizeof(BuddyBlock));

    // Validar el bloque
    if (block->blockState != ALLOCATED) {
        return NULL; // Dirección inválida o bloque no asignado
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

    return NULL;
}

static BuddyBlock* getBuddy(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS) {
        return NULL;
    }
    
    // Calculate buddy address using XOR
    uint64_t blockAddr = (uint64_t)block;
    uint64_t buddyAddr = blockAddr ^ (1ULL << block->level);
    
    // Verify buddy is within managed memory
    if (buddyAddr < (uint64_t)memoryManager->managedMemory || 
        buddyAddr >= (uint64_t)memoryManager->managedMemory + memoryManager->memorySize) {
        return NULL;
    }
    
    return (BuddyBlock*)buddyAddr;
}

static void mergeBlocks(MemoryManagerADT memoryManager, BuddyBlock* block) {
    if (block == NULL || block->level >= BLOCKS || block->blockState != FREE) {
        return;
    }
    
    // Encontrar el buddy
    BuddyBlock* buddy = getBuddy(memoryManager, block);
    
    // Si el buddy está libre, fusionarlos
    if (buddy != NULL && buddy->blockState == FREE && buddy->level == block->level) {
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
        
        // Determinar cuál es el bloque padre (el de menor dirección)
        BuddyBlock* parent = (block < buddy) ? block : buddy;
        
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

void getMemoryInfo(memInfo *info) {
    if (info == NULL) {
        return;
    }
    info->total = memoryManager->memorySize;
    info->used = memoryManager->usedMemory;
    info->free = info->total - info->used;
}

#endif