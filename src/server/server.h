#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../utils/query.h"
#include "../query_engine/query_engine.h"


#include "utils-server.h"
#include "TS-queue.h"

/**
 * @brief Makes a socket file descriptor non-blocking.
 * 
 * @param sock The socket file descriptor to make non-blocking
 */
void make_non_blocking(int32_t sock);

/**
 * @brief High-performance epoll-based server for handling trade data queries.
 * 
 * The EpollServer class implements an asynchronous, multi-threaded server that
 * uses Linux epoll for efficient I/O multiplexing. It processes trade data queries
 * from multiple clients concurrently using a thread pool architecture with
 * lock-free queues for work distribution and result collection.
 * 
 * Key features:
 * - Non-blocking I/O with edge-triggered epoll
 * - Thread pool for query processing
 * - Asynchronous result sending with buffering
 * - Pipelined query processing
 */

class EpollServer {
 public:
  /**
   * @brief Constructs an EpollServer instance.
   * 
   * @param port The port number to bind the server to
   * @param num_worker_threads The number of worker threads to create for processing queries
   */

  EpollServer(int32_t port, int32_t num_worker_threads);
  /**
   * @brief Destructor that cleans up server resources.
   * 
   * Closes all open file descriptors and releases memory.
   */
  ~EpollServer();
  
  /**
   * @brief Starts the main server event loop.
   * 
   * This method blocks and runs the server's main event loop, handling incoming
   * connections, client requests, and outgoing responses until the server is stopped.
   */
  void run();
 
 private:
  // Server socket address configuration
  sockaddr_in server_address_;
  // Server listening socket file descriptor
  int32_t server_listen_fd_;
  // Epoll instance file descriptor for I/O multiplexing
  int32_t epoll_fd_;
  /**
   * @brief Adds a socket to the epoll instance with specified events.
   * 
   * @param sock The socket file descriptor to add
   * @param events The epoll events to monitor (EPOLLIN, EPOLLOUT, etc.)
   */
  void add_to_epoll(int32_t sock, uint32_t events);
  /**
   * @brief Binds the server socket to the configured address and port.
   */
  void bind_server();
  std::queue<std::pair<int32_t, TradeDataQuery>> task_queue_;
  // Pool of worker threads for processing queries
  std::vector<std::thread> worker_threads_;
  // Thread-safe queue for distributing work items to worker threads
  TSQueue<WorkItem> work_queue_;
  // Thread-safe queue for collecting results from worker threads
  TSQueue<ResultItem> results_queue_;

  /**
   * @brief Structure for managing partial writes to clients.
   * 
   * When a client's socket buffer is full, this structure tracks
   * the remaining data to be sent and the progress made so far.
   */
  struct OutgoingBuffer {
      std::vector<char> buffer;
      size_t sent_bytes = 0;
  };
  // Map of client file descriptors to their pending outgoing buffers
  std::unordered_map<int32_t, OutgoingBuffer> write_buffers_;

  /**
   * @brief Handles incoming data from a client socket.
   * 
   * Reads TradeDataQuery structures from the client and enqueues them
   * for processing by worker threads.
   * 
   * @param client_fd The client socket file descriptor
   */
  void handle_read(int32_t client_fd);
  /**
   * @brief Handles outgoing data to a client socket when it becomes writable.
   * 
   * Continues sending any buffered data that couldn't be sent previously
   * due to full socket buffers.
   * 
   * @param client_fd The client socket file descriptor
   */
  void handle_write(int32_t client_fd);
  /**
   * @brief Processes completed results from worker threads.
   * 
   * Drains the results queue and initiates sending of completed query results
   * back to the appropriate clients.
   */
  void process_results();
  /**
   * @brief Sends a query result to a client.
   * 
   * Attempts to send the result immediately, but if the client's socket buffer
   * is full, the data is buffered for later transmission.
   * 
   * @param result The result item containing client ID and query results
   */
  void send_result(ResultItem& result);
};

