/*FIFO*/
template <typename T, int SIZE>
class FIFO {
public:
    T data[SIZE];
    uint16_t front, rear;
    FIFO():front(0),rear(0) {}
    bool enqueue(T item) {
      uint16_t len = length();
      data[rear] = item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool enqueue(T *item) {
      uint16_t len = length();
      data[rear] = *item;
      rear = (rear + 1) % SIZE;
      if(length() == SIZE) return false;  //Overflow
      return true;
    }
    bool dequeue(T *item) {
      if (front == rear) return false;
      *item = data[front];
      front = (front + 1) % SIZE;
      return true;
    }
    bool dequeue() {
      if (front == rear) return false;
      front = (front + 1) % SIZE;
      return true;
    }
    inline int length() {
      return rear>=front?rear-front:SIZE-front+rear;
    }
};