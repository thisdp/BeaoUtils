#pragma once

#define CAST(OBJ,typeName) (*((typeName*)(&OBJ)))
#define pCAST(OBJ,typeName) ((typeName*)OBJ)

class MemoryController {
public:
    size_t heapHead;
    size_t heapPointer;
    size_t heapSize;
    MemoryController(size_t hp, size_t size = 4096) : heapPointer(hp), heapSize(size), heapHead(hp) {}
    void* malloc(size_t size) {
        if (size > 4) {
            size_t alignment = 4; // 4字节对齐
            heapPointer = (heapPointer + (alignment - 1)) & ~(alignment - 1);
        }
        if (heapPointer + size >= heapHead+heapSize) return 0;  //Out of memory
        void* ptr = (void*)heapPointer;
        heapPointer += size;
        memset(ptr, 0, size);
        return ptr;
    }
    template<typename T>
    T* alloc(size_t count = 1) {
        return (T*)malloc(sizeof(T) * count);
    }
    template<typename T>
    T& allocref(size_t count = 1) {
        return *((T*)malloc(sizeof(T) * count));
    }
};

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