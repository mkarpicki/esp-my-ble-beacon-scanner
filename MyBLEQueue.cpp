#include <Arduino.h>
#include "MyBLEQueue.h";

MyBLEQueue::MyBLEQueue(void)
{
  firstItem = NULL;
  lastItem = NULL;
  maxSize = 0;
  currentSize = 0;  
}

MyBLEQueue::~MyBLEQueue()
{
 while (!isEmpty())
  {
    pop();
  }
}

bool MyBLEQueue::push(MyBLEBeacon * element)
{

  /* create new item with delivered value */
  QueueItem * newItem = new QueueItem(element);

  /* if there was last item already (not empty case)
  * point it's next value to new item
  */
  if (lastItem != NULL)
  {
    lastItem->next = newItem; 
  }

  /* if queue was empty 
  * remember new item as fisrt
  */
  if (firstItem == NULL)
  {
    firstItem = newItem;
  }

  /* remember new element as last one */
  lastItem = newItem;

  /* increment size of queue */
  currentSize++;

  return true;
}

MyBLEBeacon * MyBLEQueue::pop()
{
  /* if queue is empty, return */
  if (firstItem == NULL)
  {
    return NULL;
  }

  /* get value of first item */
  MyBLEBeacon * element = new MyBLEBeacon(); 
  *element = *(firstItem->value);

  QueueItem * next = firstItem->next;

  delete firstItem;

  /* remember next one as new first item*/
  firstItem = next;

  /* decrease size */
  currentSize--;

  /* return value kept in element that was first */
  return element;
}


bool MyBLEQueue::isEmpty()
{
  return (firstItem == NULL) ? true : false;
}

int MyBLEQueue::getSize()
{
  return currentSize;
}

/*******************************/
// private sub class
/******************************/
MyBLEQueue::QueueItem::QueueItem()
{

}

MyBLEQueue::QueueItem::QueueItem(MyBLEBeacon * element)
{
  value = new MyBLEBeacon();
  *value = *element;
  next = NULL;
}


MyBLEQueue::QueueItem::~QueueItem()
{
  delete value;
}
