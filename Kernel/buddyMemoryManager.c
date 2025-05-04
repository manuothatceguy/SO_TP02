#ifdef buddy
#include <memoryManager.h>

#define BUDDY_MIN_BLOCK_SIZE 8 // 2^3 = 8 bytes
#define BLOCKS 30 // 2^30 = 1GB

enum state {FREE, SPLIT, ALLOCATED};

typedef struct BuddyBlock {
    uint8_t level;
    enum state blockState;
    struct BuddyBlock * next;
} BuddyBlock;

typedef struct MemoryManagerCDT {
    void *memoryForMemoryManager;
    void *managedMemory;
    uint64_t memorySize;
    BuddyBlock * blocks[BLOCKS];
} MemoryManagerCDT;

uint64_t logn(uint64_t x, uint64_t n) {
    uint8_t result = 0;
    while (x /= n) {
        result++;
    }
    return result;
}

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, uint64_t memorySize ) {
    MemoryManagerADT memoryManager = (memoryManagerADT) memoryForMemoryManager;
    memoryManager->memoryForMemoryManager = memoryForMemoryManager;
    memoryManager->managedMemory = managedMemory;
    memoryManager->memorySize = memorySize;
    uint64_t exponent = logn(memorySize,2); // validar etc

    for (uint64_t i = 0; i < exponent; i++) {
        memoryManager->blocks[i] = NULL;
    }    

    
    
}

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate){
    uint64_t exponent = logn(memoryToAllocate,2);
    if (exponent < 3) {
        exponent = 3;
    }
    if (exponent > BLOCKS) {
        return NULL;
    }
    BuddyBlock * block = memoryManager->levels[exponent];

}

static void initMemoryBlock(BuddyBlock * block, uint8_t level){
    block->level = level;
    block->blockState = FREE;
    block->next = NULL;
}

#endif