#include "MemoryController.h"
MemoryController::MemoryController(size_t hp, size_t size) :
    heapHead(hp),
    heapPointer(hp),
    heapSize(size){}

void* MemoryController::malloc(size_t size) {
    if (size > 4) {
        size_t alignment = 4; // 4字节对齐
        heapPointer = (heapPointer + (alignment - 1)) & ~(alignment - 1);
    }
    if (heapPointer + size >= heapHead+heapSize) return nullptr;  // Out of memory
    void* ptr = (void*)heapPointer;
    heapPointer += size;
    memset(ptr, 0, size);
    return ptr;
}

// Placement new operators
void* operator new(size_t size, MemoryController& m) {
    return m.malloc(size);
}

void* operator new[](size_t size, MemoryController& m) {
    return m.malloc(size);
}
/*
Example:
//暂时未实现动态内存管理
MemoryController m(0x1000);
int *a = new(m) int(10);
int *b = m.alloc<int>(10);
int &c = m.allocref<int>(10);
uint8_t *data = m.malloc(1024);
*/