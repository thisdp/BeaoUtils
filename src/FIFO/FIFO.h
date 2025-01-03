#pragma once
/*FIFO*/
template <typename T, int SIZE>
class FIFO {
public:
    T data[SIZE];
    uint16_t front;
    uint32_t len;
    bool overwriteOnFull;
    FIFO():front(0),rear(0), overwriteOnFull(false) {}
    void setOverwriteOnFull(bool value) {
      overwriteOnFull = value;
    }
    bool enqueue(T& item) {
        if (isFull()) {
            if (!overwriteOnFull) return false; // Overflow
            dequeue();
        }
        data[getRear()] = item;
        ++len;
        return true;
    }
    bool enqueue(T* item) {
        if (item == nullptr) return false;
        if (isFull()) {
            if (!overwriteOnFull) return false; // Overflow
            dequeue();
        }
        data[getRear()] = *item;
        ++len;
        return true;
    }
    bool dequeue(T** item) {
        if (isEmpty()) return false;
        *item = &(data[front]);
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue(T*& item) {
        if (isEmpty()) return false;
        item = &(data[front]);
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue(T& item) {
        if (isEmpty()) return false;
        item = data[front];
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue() {
        if (isEmpty()) return false;
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool peek(T& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        item = data[(front + index) % SIZE];
        return true;
    }
    bool peek(T*& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        item = &(data[(front + index) % SIZE]);
        return true;
    }
    bool peek(T**& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        *item = &(data[(front + index) % SIZE]);
        return true;
    }
    inline uint32_t length() const { return len; }
    inline bool isFull() const { return len == SIZE; }
    inline bool isEmpty() const { return len == 0; }
    inline int emptyLength() const { return SIZE - len; }
};



template <typename T>
class DynamicFIFO {
private:
    T* data;
    uint32_t front;
    uint32_t len;
    uint32_t SIZE;
    bool overwriteOnFull;
    inline uint32_t getRear() const { return (front + len) % SIZE; }
public:
    DynamicFIFO(uint32_t size) : front(0), len(0), overwriteOnFull(false) {
        SIZE = size;
        data = new T[SIZE];
    }
    ~DynamicFIFO() { delete[] data; }
    void setOverwriteOnFull(bool value) {
        overwriteOnFull = value;
    }
    void resize(uint32_t newSize) {
        if (newSize == SIZE) return;
        T* newData = new T[newSize];
        for (uint32_t i = 0; i < len; ++i) {
            newData[i] = data[(front + i) % SIZE];
        }
        delete[] data;
        data = newData;
        SIZE = newSize;
        front = 0; // Reset front after resizing
        if (len > newSize) {
            len = newSize;
        }
    }
    bool enqueue(T& item) {
        if (isFull()) {
            if (!overwriteOnFull) return false; // Overflow
            dequeue();
        }
        data[getRear()] = item;
        ++len;
        return true;
    }
    bool enqueue(T* item) {
        if (item == nullptr) return false;
        if (isFull()) {
            if (!overwriteOnFull) return false; // Overflow
            dequeue();
        }
        data[getRear()] = *item;
        ++len;
        return true;
    }
    bool dequeue(T** item) {
        if (isEmpty()) return false;
        *item = &(data[front]);
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue(T*& item) {
        if (isEmpty()) return false;
        item = &(data[front]);
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue(T& item) {
        if (isEmpty()) return false;
        item = data[front];
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool dequeue() {
        if (isEmpty()) return false;
        front = (front + 1) % SIZE;
        --len;
        return true;
    }
    bool peek(T& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        item = data[(front + index) % SIZE];
        return true;
    }
    bool peek(T*& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        item = &(data[(front + index) % SIZE]);
        return true;
    }
    bool peek(T**& item, uint32_t index) const { // 类似于dequeue，但是不会改变front指针
        if (isEmpty() || index >= len) return false;
        *item = &(data[(front + index) % SIZE]);
        return true;
    }
    inline uint32_t length() const { return len; }
    inline bool isFull() const { return len == SIZE; }
    inline bool isEmpty() const { return len == 0; }
    inline int emptyLength() const { return SIZE - len; }
};