#include "mt_queue.h"

std::unique_ptr<T> Mt_Queue::pop()
{
  qsize.acquire();
  qlock.acquire();
  std::unique_ptr<T> ptr = std::move(_queue.front());
  _queue.pop();
  qlock.release();
  return ptr;
}

void Mt_Queue::push(std::unique_ptr<T> ptr)
{
  qlock.acquire();
  _queue.push(std::move(ptr));
  qlock.release();
  qsize.release();
}