#pragma once
/*FIFO*/
template <typename T, int SIZE>
class FIFO {
public:
    T data[SIZE];
    uint16_t front, rear;
    bool overwriteOnFull;
    FIFO():front(0),rear(0), overwriteOnFull(false) {}
    void setOverwriteOnFull(bool value) {
      overwriteOnFull = value;
    }
    bool enqueue(T &item) {
      if(length() == SIZE){
        if (overwriteOnFull) {
          front = (front + 1) % SIZE; // Overwrite the oldest item
        } else {
          return false;  // Overflow
        }
      }
      data[rear] = item;
      rear = (rear + 1) % SIZE;
      return true;
    }
    bool enqueue(T *item) {
      if(item == 0) return false; //Not intend to use nullptr
      if(length() == SIZE){
        if (overwriteOnFull) {
          front = (front + 1) % SIZE; // Overwrite the oldest item
        } else {
          return false;  // Overflow
        }
      }
      data[rear] = *item;
      rear = (rear + 1) % SIZE;
      return true;
    }
    bool dequeue(T **item) {
      if (isEmpty()) return false;
      *item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T *&item){
      if (isEmpty()) return false;
      item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T &item) {
      if (isEmpty()) return false;
      item = data[front];
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue() {
      if (isEmpty()) return false;
      front = (front + 1) % SIZE;
      return true;
    }
    bool peek(T &item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if(index >= length()) return false;
      item = data[(front+index)%SIZE];
      return true;
    }
    bool peek(T *&item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if (index >= length()) return false;
      item = &(data[(front+index)%SIZE]);
      return true;
    }
    bool peek(T **&item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if (index >= length()) return false;
      *item = &(data[(front+index)%SIZE]);
      return true;
    }
    inline int length() { return rear>=front?rear-front:SIZE-front+rear; }
    inline bool isFull(){ return length() == SIZE; }
    inline bool isEmpty(){ return length() == 0; }
    inline int emptyLength(){ return SIZE - length(); }
};

template <typename T>
class DynamicFIFO {
public:
    T *data;
    uint16_t front, rear;
    uint32_t SIZE;
    bool overwriteOnFull;
    DynamicFIFO(uint32_t size):front(0),rear(0),overwriteOnFull(false) { SIZE = size; data = new T[SIZE]; }
    void setOverwriteOnFull(bool value) {
      overwriteOnFull = value;
    }
    ~DynamicFIFO() { delete[] data; }
    void resize(uint32_t size) {
      if(size == SIZE) return;
      T *newData = new T[size];
      uint32_t len = length();
      for(uint32_t i = 0; i < len; i++) {
        newData[i] = data[(front+i)%SIZE];
      }
      delete[] data;
      data = newData;
    }
    bool enqueue(T &item) {
      if(length() == SIZE){
        if (overwriteOnFull) {
          front = (front + 1) % SIZE; // Overwrite the oldest item
        } else {
          return false;  // Overflow
        }
      }
      data[rear] = item;
      rear = (rear + 1) % SIZE;
      return true;
    }
    bool enqueue(T *item) {
      if(item == 0) return false; //Not intend to use nullptr
      if(length() == SIZE){
        if (overwriteOnFull) {
          front = (front + 1) % SIZE; // Overwrite the oldest item
        } else {
          return false;  // Overflow
        }
      }
      data[rear] = *item;
      rear = (rear + 1) % SIZE;
      return true;
    }
    bool dequeue(T **item) {
      if (isEmpty()) return false;
      *item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T *&item){
      if (isEmpty()) return false;
      item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T &item) {
      if (isEmpty()) return false;
      item = data[front];
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue() {
      if (isEmpty()) return false;
      front = (front + 1) % SIZE;
      return true;
    }
    bool peek(T &item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if(index >= length()) return false;
      item = data[(front+index)%SIZE];
      return true;
    }
    bool peek(T *&item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if (index >= length()) return false;
      item = &(data[(front+index)%SIZE]);
      return true;
    }
    bool peek(T **&item, uint32_t index){ //类似于dequeue，但是不会改变front指针
      if (isEmpty()) return false;
      if (index >= length()) return false;
      *item = &(data[(front+index)%SIZE]);
      return true;
    }
    inline int length() { return rear>=front?rear-front:SIZE-front+rear; }
    inline bool isFull(){ return length() == SIZE; }
    inline bool isEmpty(){ return length() == 0; }
    inline int emptyLength(){ return SIZE - length(); }
};