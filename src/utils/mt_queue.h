#pragma once
#include <memory>
#include <mutex>
#include <semaphore>
#include <queue>

#define LEAST_MAX_QSIZE 10000

template <typename T>
class Mt_Queue
{
public:
  Mt_Queue() : qsize(0) {}

  std::unique_ptr<T> pop()
  {
    qsize.acquire();
    qlock.lock();
    std::unique_ptr<T> ptr = std::move(_queue.front());
    _queue.pop();
    qlock.unlock();
    return ptr;
  }

  void push(std::unique_ptr<T> ptr)
  {
    qlock.lock();
    _queue.push(std::move(ptr));
    qlock.unlock();
    qsize.release();
  }

  bool empty()
  {
    qlock.lock();
    bool ret = _queue.empty();
    qlock.unlock();
    return ret;
  }

private:
  std::mutex qlock;
  std::counting_semaphore<LEAST_MAX_QSIZE> qsize;
  std::queue<std::unique_ptr<T>> _queue;
};
