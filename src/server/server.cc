#include "server.h"
#include "../query_engine/query_engine.h"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"
#include "../utils/query.h"
#include "sender.h"

#include <spdlog/spdlog.h>
#include <thread>
#include <vector>


void worker_thread_loop(TSQueue<WorkItem>& work_queue, TSQueue<ResultItem>& results_queue);

void worker_thread_loop(TSQueue<WorkItem>& work_queue, TSQueue<ResultItem>& results_queue) {
  Query_engine exec; // Each thread has its own Executor instance

  while (true) {
    WorkItem work = work_queue.pop();

    ResultItem result_item;
    result_item.client_fd = work.client_fd;
    int8_t metric_list = 0;
    if (work.query.metrics & (1 << 0)) {
        metric_list |= 1 << 0; // min and max price
    }
    if (work.query.metrics & (1 << 26)) {
        metric_list |= 1 << 1; // mean price
    }
    if (work.query.metrics & (1ull << 33)) {
        metric_list |= 1 << 2; // total quantity
    }
    
    metric_list = 7; // Assuming we want to compute all metrics (00000111), comment this line out later

    spdlog::info("Worker thread processing query for client {} with metrics: {}", work.client_fd, metric_list);

    std::vector<char> accumulated_buffer;

    for (uint64_t start_time = work.query.start_time_point; start_time < work.query.end_time_point; start_time += work.query.resolution) {
        uint64_t end_time = start_time + work.query.resolution;
        if (end_time > work.query.end_time_point) {
            end_time = work.query.end_time_point;
        }

    // Create a temporary query for this single interval
    TradeDataQuery interval_query = work.query;
    interval_query.start_time_point = start_time;
    interval_query.end_time_point = end_time;

    // Calculate results for this interval only
    std::vector<char> interval_result = exec.aggregator(metric_list, interval_query);
    
    // Append to accumulated buffer
    accumulated_buffer.insert(accumulated_buffer.end(), interval_result.begin(), interval_result.end());
    }

    // Send all intervals as one result
    // ResultItem result_item;
    result_item.client_fd = work.client_fd;
    result_item.res = std::move(accumulated_buffer);
    results_queue.push(std::move(result_item));
    }

  }

constexpr int32_t MAX_EVENTS = 10;

EpollServer::EpollServer(int32_t port, int32_t num_worker_threads)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_fd_ = epoll_create1(0)) {
  int32_t opt = 1;
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEADDR");
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEPORT");
  make_non_blocking(server_listen_fd_);
  bind_server();
  add_to_epoll(server_listen_fd_, EPOLLIN);
  spdlog::info("Starting {} worker threads.", num_worker_threads);
    for (int i = 0; i < num_worker_threads; ++i) {
        worker_threads_.emplace_back(worker_thread_loop, std::ref(work_queue_), std::ref(results_queue_));
  }
}

EpollServer::~EpollServer()
{
  close(server_listen_fd_);
  close(epoll_fd_);
  spdlog::info("Server ended");
}

void EpollServer::run()
{
  helper::check_error(listen(server_listen_fd_, SOMAXCONN) < 0,
                      "Failed to listen on server socket");
  epoll_event events[MAX_EVENTS];
  while (true) {
    int32_t n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);
    helper::check_error(n == -1, "epoll_wait failed");

    for (int32_t i = 0; i < n; ++i)
    {
      if (events[i].data.fd == server_listen_fd_)
      {
        // Accept all incoming connections
        while (true)
        {
          int32_t client_fd = accept(server_listen_fd_, nullptr, nullptr);
          if (client_fd == -1)
          {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            helper::check_error(true, "Failed to accept connection");
          }
          make_non_blocking(client_fd);
          add_to_epoll(client_fd, EPOLLIN);
        }
      } else if (events[i].events & EPOLLIN) {
        // Read data from client into the work queue
        handle_read(events[i].data.fd);
      }
      else if (events[i].events & EPOLLOUT) {
        // Handle write events
        handle_write(events[i].data.fd);
        
      } 
      else {
        spdlog::warn("Unexpected event on fd {}: {}", events[i].data.fd, events[i].events);
      }
    }
    process_results();
  }
}


void EpollServer::add_to_epoll(int32_t sock, uint32_t events) {
  epoll_event event{};
  event.data.fd = sock;
  event.events = events;

  helper::check_error(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock, &event) == -1,
                      "Failed to add socket to epoll");
  spdlog::info("Socket {} added to epoll.", sock);
}

void make_non_blocking(int32_t sock)
{
  int32_t flags = fcntl(sock, F_GETFL, 0);
  helper::check_error(flags == -1, "Failed to get socket flags");
  helper::check_error(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1,
                      "Failed to set socket to non-blocking");
  spdlog::info("Server switched socket {} to non-blocking mode.", sock);
}

void EpollServer::bind_server()
{
  helper::check_error(
      bind(server_listen_fd_, reinterpret_cast<sockaddr *>(&server_address_),
           sizeof(server_address_)) < 0,
      "Failed to bind server socket");
  spdlog::info("Server bound to port {}", ntohs(server_address_.sin_port));
}


void EpollServer::handle_read(int32_t client_fd) {
    while (true) {
        TradeDataQuery query;
        ssize_t count = recv(client_fd, &query, sizeof(query), 0);
        if (count > 0) {
            spdlog::info("Received query from client {}. Offloading to worker.", client_fd);
            work_queue_.push({client_fd, query});
            // If you expect multiple queries per event, continue the loop.
            // Otherwise, break here if only one query per read is expected.
        } else if (count == 0) {
            // Client closed connection
            spdlog::info("Client {} disconnected.", client_fd);
            close(client_fd);
            write_buffers_.erase(client_fd);
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data to read
                break;
            } else {
                // Real error
                spdlog::error("recv error on client {}: {}", client_fd, strerror(errno));
                close(client_fd);
                write_buffers_.erase(client_fd);
                break;
            }
        }
    }
}


// This is called in every loop to check for finished work
void EpollServer::process_results() {
    ResultItem result;
    while (results_queue_.try_pop(result)) {

    spdlog::info("Processing result for client {}", result.client_fd);
  

    // Now we have a result. We need to send it back.
    send_result(result);
    }
}
 
// New function to handle sending and buffering
void EpollServer::send_result(ResultItem& result) {
  // 1. Serialize the result into a byte buffer
  std::vector<char> buffer;
  buffer = std::move(result.res); // Move the serialized data into the buffer

  int32_t count = static_cast<int32_t>(buffer.size());
  ssize_t bytes_sent = send(result.client_fd, &count, sizeof(count), 0);
  if (bytes_sent < 0) {
      perror("send (count)");
      close(result.client_fd);
      return;
  }

  // 2. Then send the actual buffer
  bytes_sent = send(result.client_fd, buffer.data(), buffer.size(), 0);
  if (bytes_sent < 0) {
      perror("send (buffer)");
      close(result.client_fd);
      return;
  }
    if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Kernel buffer is full, need to wait
            bytes_sent = 0; // Treat as if nothing was sent yet
        } else {
            // Real error
            perror("send");
            close(result.client_fd);
            return;
        }
    }

    // 3. If not all data was sent, buffer the rest and wait for EPOLLOUT
    if (static_cast<size_t>(bytes_sent) < buffer.size()) {
        spdlog::warn("Partial send for client {}. Buffering remaining data.", result.client_fd);
        write_buffers_[result.client_fd] = {std::move(buffer), static_cast<size_t>(bytes_sent)};

        // Modify epoll to watch for writability
        epoll_event event{};
        event.data.fd = result.client_fd;
        event.events = EPOLLIN | EPOLLOUT | EPOLLET; // Watch for read AND write
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, result.client_fd, &event);
    }
}

// This is called when the socket is ready for writing again
void EpollServer::handle_write(int32_t client_fd) {
    spdlog::info("Handling write for client {}", client_fd);
    auto it = write_buffers_.find(client_fd);
    if (it == write_buffers_.end()) {
        spdlog::warn("No write buffer found for client {}", client_fd);
        return; // Should not happen
    }

    OutgoingBuffer& ob = it->second;
    ssize_t bytes_sent = send(client_fd, ob.buffer.data() + ob.sent_bytes, ob.buffer.size() - ob.sent_bytes, 0);
    spdlog::info("Attempting to send {} bytes to client {}", ob.buffer.size() - ob.sent_bytes, client_fd);
    if (bytes_sent >= 0) {
        spdlog::info("Sent {} bytes to client {}", bytes_sent, client_fd);
        ob.sent_bytes += bytes_sent;
        if (ob.sent_bytes == ob.buffer.size()) {
            // All data sent!
            spdlog::info("Finished sending buffered data to client {}.", client_fd);
            write_buffers_.erase(it);

            // Modify epoll to stop watching for writability
            epoll_event event{};
            event.data.fd = client_fd;
            event.events = EPOLLIN | EPOLLET; // Go back to only watching for reads
            epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, client_fd, &event);
        }
    } else {
        // Handle error
        perror("send on handle_write");
        close(client_fd);
        write_buffers_.erase(it);
    }
}

