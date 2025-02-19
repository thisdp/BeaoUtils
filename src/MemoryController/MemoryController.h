#pragma once
#include "Arduino.h"
#include <string.h>
#define CAST(OBJ,typeName) (*((typeName*)(&OBJ)))
#define pCAST(OBJ,typeName) ((typeName*)OBJ)
using namespace std;

class MemoryController {
public:
    // Constructor declaration
    MemoryController(size_t hp, size_t size = 4096);

    // Method declarations
    void* malloc(size_t size);
    
    template<typename T>
    T* alloc(size_t count = 1);

    template<typename T>
    T& allocref(size_t count = 1);

    // Member variables
    size_t heapHead;
    size_t heapPointer;
    size_t heapSize;
};

// Placement new operators
void* operator new(size_t size, MemoryController& m);
void* operator new[](size_t size, MemoryController& m);

/*
Example:
//暂时未实现动态内存管理
MemoryController m(0x1000);
int *a = new(m) int(10);
int *b = m.alloc<int>(10);
int &c = m.allocref<int>(10);
uint8_t *data = m.malloc(1024);
*/