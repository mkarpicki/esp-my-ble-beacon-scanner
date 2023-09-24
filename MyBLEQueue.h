#include "MyBLEBeacon.h"

#ifndef MyBLEQueue_h
#define MyBLEQueue_h

class MyBLEQueue
{
  public:
    MyBLEQueue();
    ~MyBLEQueue();
  
    bool push(MyBLEBeacon *);
    MyBLEBeacon * pop();
    bool isEmpty();

  private:
  
    class QueueItem
    {
    public:
      QueueItem();
      QueueItem(MyBLEBeacon *);
      ~QueueItem();
  
      MyBLEBeacon * value;
      QueueItem * next;
    };
  
    QueueItem * firstItem;
    QueueItem * lastItem;
  
    int maxSize;
    int currentSize;
};

#endif    
