#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct TestCase {
  std::string id;
  std::string request;
  std::string expected_reply;
  std::string error_msg;
};

struct ClientState {
  int fd{-1};
  bool awaiting_reply{false};
  std::chrono::steady_clock::time_point send_ts{};
  std::chrono::nanoseconds latency{};
  std::string expected_reply;
  std::string error_msg;
};

class AntiServer {
public:
  explicit AntiServer(std::string_view host, uint16_t port,
                      int num_clients = 256);

  ~AntiServer();
  void run();

private:
  void load_testcases(const std::string &path);
  void prepare_clients();
  void send_next_case(ClientState &c, const TestCase &tc);
  void process_epoll_events();

  void add_fd_to_epoll(int fd);
  void handle_readable(ClientState &c);
  void report_results() const;

  std::string host_;
  uint16_t port_;
  int epoll_fd_{-1};
  std::vector<TestCase> cases_;
  std::vector<ClientState> clients_;
  std::unordered_map<int, size_t> fd_to_idx_;
};
