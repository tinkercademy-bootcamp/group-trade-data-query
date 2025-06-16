#include <memory>
#include <mutex>
#include <semaphore>

template <typename T>
class Mt_Queue
{
public:
  std::unique_ptr<T> pop();
  void push(std::unique_ptr<T>);

private:
  std::mutex qlock;
  std::counting_semaphore qsize;
  std::queue<std::unique_ptr<T>> _queue;
};