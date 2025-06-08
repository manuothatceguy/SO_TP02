#ifdef BITMAP
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <textModule.h>
#include <debug.h>

enum { FREE, USED, START_BOUNDARY, END_BOUNDARY, SINGLE_BLOCK };

typedef struct mm {
  void *start;
  uint32_t blockQty;
  uint32_t blocksUsed;
  uint32_t *bitmap;
  uint32_t current;
} mm;

mm memoryManager;

static void initializeBitmap();
static uint32_t sizeToBlockQty(uint32_t size);

void createMemoryManager() {
  uint32_t totalNeeded = HEAP_SIZE;
  memoryManager.start = (void*)HEAP_START_ADDRESS;
  memoryManager.blockQty = totalNeeded / BLOCK_SIZE;
  if (totalNeeded % BLOCK_SIZE != 0) {
    memoryManager.blockQty++;
    totalNeeded = BLOCK_SIZE * memoryManager.blockQty;
  }
  uint32_t bitMapSize = memoryManager.blockQty / BLOCK_SIZE;
  if (memoryManager.blockQty % BLOCK_SIZE != 0) {
    bitMapSize++;
  }
  totalNeeded += bitMapSize * BLOCK_SIZE;
//   if (totalNeeded > HEAP_SIZE) {
//     return -1;
//   }

  memoryManager.bitmap = (void*)HEAP_START_ADDRESS;
  memoryManager.blocksUsed = 0;
  memoryManager.start = memoryManager.bitmap + bitMapSize * BLOCK_SIZE;
  memoryManager.current = 0;

  initializeBitmap();
  return;
}

static uint32_t sizeToBlockQty(uint32_t size) {
  return (uint32_t)((size % BLOCK_SIZE) ? (size / BLOCK_SIZE) + 1 : size / BLOCK_SIZE);
}

static void initializeBitmap() {
  for (int i = 0; i < memoryManager.blockQty; i++) {
    memoryManager.bitmap[i] = FREE;
  }
}

static uintptr_t findFreeBlocks(uint32_t blocksNeeded, uint32_t start, uint32_t end) {
  uint32_t freeBlocks = 0;
  uint32_t i;
  for (i = start; i < end; i++) {
    if (memoryManager.bitmap[i] == FREE) {
      freeBlocks++;
      if (freeBlocks == blocksNeeded) {
        return (uintptr_t)(memoryManager.start + (i - blocksNeeded + 1) * BLOCK_SIZE);
      }
    } else {
      freeBlocks = 0;
    }
  }
  if (freeBlocks == blocksNeeded) {
    return (uintptr_t)(memoryManager.start + (i - blocksNeeded + 1) * BLOCK_SIZE);
  }
  return 0;
}

static void *markGroupAsUsed(uint32_t blocksNeeded, uint32_t index) {
  memoryManager.bitmap[index] = START_BOUNDARY;
  memoryManager.bitmap[index + blocksNeeded - 1] = END_BOUNDARY;
  memoryManager.blocksUsed += 2;
  for (int i = 1; i < blocksNeeded - 1; i++) {
    memoryManager.blocksUsed++;
    memoryManager.bitmap[index + i] = USED;
  }
  return (void *)(memoryManager.start + index * BLOCK_SIZE);
}

void *allocMemory(const size_t memoryToAllocate) {
  uint32_t blocksNeeded = sizeToBlockQty(memoryToAllocate);

  if (blocksNeeded > memoryManager.blockQty - memoryManager.blocksUsed) {
    DEBUG_PRINT("No hay suficiente memoria", DEBUG_COLOR);
    return NULL;
  }
  uintptr_t initialBlockAddress = findFreeBlocks(
      blocksNeeded, memoryManager.current, memoryManager.blockQty);

  if (initialBlockAddress == 0) {
    initialBlockAddress =
        findFreeBlocks(blocksNeeded, 0, memoryManager.blockQty);
  }
  if (initialBlockAddress == 0) {
    DEBUG_PRINT("No hay suficiente memoria",DEBUG_COLOR );
    return NULL;
  }
  memoryManager.current = sizeToBlockQty(initialBlockAddress - (uintptr_t)memoryManager.start) + blocksNeeded;
  
  if (blocksNeeded == 1) {
    memoryManager.blocksUsed++;
    memoryManager.bitmap[(initialBlockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE] = SINGLE_BLOCK;
    return (void *)initialBlockAddress;
  }

  return markGroupAsUsed( blocksNeeded, (initialBlockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE);
}

void* freeMemory(void *const restrict memory) {
  if (memory == NULL || (uintptr_t)memory % BLOCK_SIZE != 0) {
    return NULL;
  }
  uintptr_t blockAddress = (uintptr_t)memory;
  uint32_t blockIndex = (blockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE;
  if (memoryManager.bitmap[blockIndex] == SINGLE_BLOCK) {
    memoryManager.bitmap[blockIndex] = FREE;
    memoryManager.blocksUsed--;
    return NULL;
  }
  if (memoryManager.bitmap[blockIndex] != START_BOUNDARY) {
    return NULL;
  }
  uint32_t blocksToFree = 0;
  while (memoryManager.bitmap[blockIndex + blocksToFree] != END_BOUNDARY) {
    memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
    blocksToFree++;
  }
  memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
  memoryManager.blocksUsed -= blocksToFree + 1;
}
void getMemoryInfo(memInfo *info) {
    if (info == NULL) {
        return;
    }
    info->total = memoryManager.blockQty * BLOCK_SIZE;
    info->used = memoryManager.blocksUsed * BLOCK_SIZE;
    info->free = info->total - info->used;
}
#endif