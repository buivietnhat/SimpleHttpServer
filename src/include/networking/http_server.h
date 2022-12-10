
#ifndef SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
#define SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_

#include <atomic>
#include <memory>
#include <networking/socket.h>
#include <sys/epoll.h>
#include <thread>
#include <vector>

class HttpServer {
  static constexpr int BUFFER_SIZE = 2048;

  struct PeerState {
    PeerState();
    int fd;
    char buffer[BUFFER_SIZE];
    int length;
    int sendptr;
  };

  class ConnectionManager {
    static constexpr int MAX_EVENTS = 10000;

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
    void ControlEpollEvent(int epoll_fd, int op, int fd, uint32_t events, void *data);
    void ProcessEpollEvents(int worker_id);
    void ProcessEpollInEvents(int epoll_fd, PeerState *state);
    void ProcessEpollOutEvents(int epoll_fd, PeerState *state);
  };

 public:
  HttpServer(int num_worker, std::unique_ptr<Socket> &&socket);
  void Start(const std::string &host, int port);

 private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<ConnectionManager> cm_;
};

#endif//SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
