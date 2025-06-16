#include "server.h"
#include "../query_engine/query_engine.h"
#include "utils-server.h"
#include "TS-queue.h"
// #include "../../processed_data/preproc.cc"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"
#include "../utils/query.h"
#include "sender.h"
#include <spdlog/spdlog.h>

/////////////////

std::vector<TradeData> execute_task(TradeDataQuery& query) {
  // Dummy implementation: return an empty vector
  spdlog::info("Query processed for TradeDataQuery id: {}", query.symbol_id);
  return {};
}

/////////////////

void worker_thread_loop(TSQueue<WorkItem>& work_queue, TSQueue<ResultItem>& results_queue);

void worker_thread_loop(TSQueue<WorkItem>& work_queue, TSQueue<ResultItem>& results_queue) {
    Executor exec; // Each thread has its own Executor instance

    while (true) {
        WorkItem work = work_queue.pop();

        ResultItem result_item;
        result_item.client_fd = work.client_fd;

        if (work.query.resolution > 0) {
            result_item.is_trade_data = false;
            result_item.resolution_results = exec.lowest_and_highest_prices(work.query);
        } else {
            result_item.is_trade_data = true;
            result_item.trade_data_results = exec.send_raw_data(work.query);
        }

        results_queue.push(std::move(result_item));
    }
}



constexpr int32_t MAX_EVENTS = 10;

EpollServer::EpollServer(int32_t port, int32_t num_worker_threads)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_fd_ = epoll_create1(0)) {
  int opt = 1;
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

EpollServer::~EpollServer() {
  close(server_listen_fd_);
  close(epoll_fd_);
  spdlog::info("Server ended");
}

void EpollServer::run() {
  helper::check_error(listen(server_listen_fd_, SOMAXCONN) < 0,
                      "Failed to listen on server socket");
  epoll_event events[MAX_EVENTS];
  while (true) {
    int32_t n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
    helper::check_error(n == -1, "epoll_wait failed");

    for (int32_t i = 0; i < n; ++i) {
      if (events[i].data.fd == server_listen_fd_) {
        // Accept all incoming connections
        while (true) {
          int32_t client_fd = accept(server_listen_fd_, nullptr, nullptr);
          if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
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
        // handle_write(events[i].data.fd);
      } 
      else {
        spdlog::warn("Unexpected event on fd {}: {}", events[i].data.fd, events[i].events);
      }
    }
    // process_results();
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

void make_non_blocking(int32_t sock) {
  int32_t flags = fcntl(sock, F_GETFL, 0);
  helper::check_error(flags == -1, "Failed to get socket flags");
  helper::check_error(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1,
                      "Failed to set socket to non-blocking");
  spdlog::info("Server switched socket {} to non-blocking mode.", sock);
}

void EpollServer::bind_server() {
  helper::check_error(
      bind(server_listen_fd_, reinterpret_cast<sockaddr*>(&server_address_),
           sizeof(server_address_)) < 0,
      "Failed to bind server socket");
  spdlog::info("Server bound to port {}", ntohs(server_address_.sin_port));
}
