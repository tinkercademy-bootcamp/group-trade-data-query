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
    qlock.acquire();
    std::unique_ptr<T> ptr = std::move(_queue.front());
    _queue.pop();
    qlock.release();
    return ptr;
  }

  void push(std::unique_ptr<T> ptr)
  {
    qlock.acquire();
    _queue.push(std::move(ptr));
    qlock.release();
    qsize.release();
  }

private:
  std::mutex qlock;
  std::counting_semaphore<LEAST_MAX_QSIZE> qsize;
  std::queue<std::unique_ptr<T>> _queue;
};
