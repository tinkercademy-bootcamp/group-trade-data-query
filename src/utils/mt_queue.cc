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
