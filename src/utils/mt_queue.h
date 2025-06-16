#pragma once
#include <memory>
#include <mutex>
#include <semaphore>

#define LEAST_MAX_QSIZE 10000

template <typename T>
class Mt_Queue
{
public:
  std::unique_ptr<T> pop();
  void push(std::unique_ptr<T>);

private:
  std::mutex qlock;
  std::counting_semaphore<LEAST_MAX_QSIZE> qsize;
  std::queue<std::unique_ptr<T>> _queue;
};