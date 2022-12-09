//
// Created by nhatbui on 09/12/2022.
//

#ifndef SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_CONNECTION_MANAGER_H_
#define SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_CONNECTION_MANAGER_H_

#include <atomic>
#include <sys/epoll.h>
#include <thread>
#include <vector>

class ConnectionManager {
  static constexpr int MAX_EVENTS = 10000;
  static constexpr int BUFFER_SIZE = 2048;

  struct PeerState {
    PeerState();
    int fd;
    uint8_t buffer[BUFFER_SIZE];
    int length;
    int sendptr;
  };

 public:
  explicit ConnectionManager(int num_workers);
  void ListenAndProcess(int socket_fd);
  virtual ~ConnectionManager();

 private:
  int num_workers_;
  int current_worker_idx{0};
  std::vector<std::thread> workers;
  std::vector<int> worker_epoll_fd_;
  std::atomic<bool> killed_{false};
  std::thread controll_thread_;
  std::vector<epoll_event *> worker_events_;

  void SetUpWorkerEpoll();
  void DistributeWork(int socket_fd);
  void ControlEpollEvent(int epoll_fd, int op, int fd, uint32_t events,
                         void *data);
  void ProcessEvents(int worker_id);
  void ProcessEpollInEvents(int epoll_fd, PeerState *state);
  void ProcessEpollOutEvents(int epoll_fd, PeerState *state);
};

#endif//SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_CONNECTION_MANAGER_H_
