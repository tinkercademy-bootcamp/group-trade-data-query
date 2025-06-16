#include "server.h"
#include "../query_engine/query_engine.h"
// #include "../../processed_data/preproc.cc"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"
#include "../utils/query.h"
#include "sender.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>

/////////////////

std::vector<TradeData> execute_task(TradeDataQuery &query)
{
  // Dummy implementation: return an empty vector
  spdlog::info("Query processed for TradeDataQuery id: {}", query.symbol_id);
  return {};
}

/////////////////

constexpr int32_t MAX_EVENTS = 10;
#define X_WORKER_THREADS 4 // Set the desired number of worker threads

EpollServer::EpollServer(uint16_t port, const std::string& file)
    : file(file),
      server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_fd_ = epoll_create1(0)) {
  int32_t opt = 1;
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEADDR");
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEPORT");
  make_non_blocking(server_listen_fd_);
  bind_server();
  add_to_epoll(server_listen_fd_);
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

  // Start worker threads
  std::vector<std::thread> worker_threads;
  for (int i = 0; i < X_WORKER_THREADS; ++i) {
    worker_threads.emplace_back(&EpollServer::query_worker, this);
  }
  // Start sender thread
  std::thread sender_thread(&EpollServer::sender_thread, this);

  while (true)
  {
    int32_t n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
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
          add_to_epoll(client_fd);
        }
      }
      else if (events[i].events & EPOLLIN)
      {
        // Read data from client
        TradeDataQuery query;
        while (true)
        {
          ssize_t count = recv(events[i].data.fd, &query, sizeof(query), 0);
          if (count == -1)
          {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            close(events[i].data.fd);
            break;
          }
          else if (count == 0)
          {
            // Connection closed
            spdlog::info("Connection closed by client fd {}", static_cast<int32_t>(events[i].data.fd));
            close(events[i].data.fd);
            break;
          }
          else
          {
            spdlog::info("Processing query from client fd {}", static_cast<int32_t>(events[i].data.fd));
            helper::check_error(
                handle_trade_data_query(events[i].data.fd, query) < 0,
                "Failed to handle the query");
          }
        }
      }
    }
  }

  // Join worker threads and sender thread (unreachable unless loop exits)
  for (auto& t : worker_threads) {
    if (t.joinable()) t.join();
  }
  if (sender_thread.joinable()) sender_thread.join();
}

int32_t EpollServer::handle_trade_data_query(int32_t sock, TradeDataQuery query)
{
  // Change: push a unique_ptr of the pair
  task_queue_.push(std::make_unique<std::pair<int32_t, TradeDataQuery>>(sock, query));
  return 0;
}

void EpollServer::query_worker()
{
  Query_engine exec(file);
  while(true)
  {
    // Change: get unique_ptr from queue and access the pair via pointer
    auto task_ptr = task_queue_.pop();
    auto &[client_sock, task_query] = *task_ptr;
    std::unique_ptr<std::pair<int32_t, std::vector<Result>>> result = std::make_unique<std::pair<int32_t, std::vector<Result>>>();
    result->first = client_sock;
    
    result->second = exec.lowest_and_highest_prices(task_query);
    send_queue_.push(std::move(result));
  }
}

void EpollServer::sender_thread()
{
  while(true)
  {
    auto send_ptr = send_queue_.pop();
    int32_t result_size = static_cast<int32_t>(send_ptr->second.size());
    int client_sock = send_ptr->first;
    bool t_not_r=false;
    // First, send the size of the result vector
    ssize_t bytes_sent =
        send(client_sock, &result_size, sizeof(result_size), 0);
    if (bytes_sent < 0)
    {
      throw std::runtime_error("Failed to send result size: " +
                               std::string(strerror(errno)));
    }
    else if (bytes_sent < static_cast<ssize_t>(sizeof(result_size)))
    {
      std::cerr << "Warning: Only " << bytes_sent << " out of "
                << sizeof(result_size) << " bytes sent for result size."
                << std::endl;
    }

    
    for (auto& data : send_ptr->second) {
      send_without_serialisation(client_sock, data);
    }
  }
}

void EpollServer::accept_connection()
{
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int32_t client_fd =
      accept(server_listen_fd_, reinterpret_cast<sockaddr *>(&client_addr),
             &client_len);
  helper::check_error(client_fd < 0, "Failed to accept connection");
  make_non_blocking(client_fd);
  add_to_epoll(client_fd);

  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
  int32_t client_port = ntohs(client_addr.sin_port);
  spdlog::info("Accepted connection from {}:{}. Assigned fd: {}", client_ip, client_port, client_fd);
}

void EpollServer::add_to_epoll(int32_t sock)
{
  epoll_event event{};
  event.data.fd = sock;
  event.events = EPOLLIN | EPOLLET; // Edge-triggered, read events

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
