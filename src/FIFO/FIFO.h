#pragma once
/*FIFO*/
template <typename T, int SIZE>
class FIFO {
public:
    T data[SIZE];
    uint16_t front, rear;
    FIFO():front(0),rear(0) {}
    bool enqueue(T &item) {
      data[rear] = item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool enqueue(T *item) {
      if(item == 0) return false; //Not intend to use nullptr
      data[rear] = *item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool dequeue(T **item) {
      if (isEmpty()) return false;
      *item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T *item){
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
    inline int length() { return rear>=front?rear-front:SIZE-front+rear; }
    inline bool isFull(){ return length() == SIZE; }
    inline bool isEmpty(){ return isEmpty(); }
    inline int emptyLength(){ return SIZE - length(); }
};


template <typename T>
class DynamicFIFO {
public:
    T *data;
    uint16_t front, rear;
    uint32_t SIZE;
    DynamicFIFO(uint32_t size):front(0),rear(0) { SIZE = size; data = new T[SIZE]; }
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
      data[rear] = item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool enqueue(T *item) {
      if(item == 0) return false; //Not intend to use nullptr
      data[rear] = *item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool dequeue(T **item) {
      if (isEmpty()) return false;
      *item = &(data[front]);
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue(T *item){
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
    inline int length() { return rear>=front?rear-front:SIZE-front+rear; }
    inline bool isFull(){ return length() == SIZE; }
    inline bool isEmpty(){ return length() == 0; }
    inline int emptyLength(){ return SIZE - length(); }
};