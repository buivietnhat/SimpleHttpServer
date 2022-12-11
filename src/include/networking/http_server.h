
#ifndef SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
#define SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_

#include "networking/http_message_define.h"
#include "networking/socket.h"
#include <atomic>
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <thread>
#include <vector>

class HttpServer {
  static constexpr size_t BUFFER_SIZE = 4096;
  static constexpr int NUM_WORKER = 5;

  struct PeerState {
    PeerState();
    int fd;
    size_t length;
    size_t sendptr;
    char buffer[BUFFER_SIZE];
  };

  class MessageParser {
   public:
    MessageParser() = default;
    static auto ToHttpRequest(const std::string &raw) -> HttpRequest;
    static auto ToPeerState(int fd, const HttpResponse &response, bool content_included) -> PeerState *;
  };

  struct Router {
    std::unordered_map<std::string, std::unordered_map<HttpMethod, std::function<HttpResponse(void)>>> route_;
    void Register(const std::string &path, HttpMethod method, std::function<HttpResponse(void)> handler);
    auto Serve(const std::string &path, const HttpRequest &request) -> HttpResponse;
  };

  class ConnectionManager {
    static constexpr int MAX_EVENTS = 10000;

   public:
    explicit ConnectionManager(int num_workers, std::unique_ptr<Router> &&router);
    void Register(const std::string &path, HttpMethod method, std::function<HttpResponse(void)> handler);
    void ListenAndProcess(int socket_fd);
    virtual ~ConnectionManager();

   private:
    int num_workers_;
    int current_worker_idx{0};
//    std::vector<std::thread> workers_;
    std::thread workers_[NUM_WORKER];
//    std::vector<int> worker_epoll_fd_;
    int worker_epoll_fd_[NUM_WORKER];
    std::atomic<bool> killed_{false};
    std::thread controll_thread_;
//    std::vector<epoll_event *> worker_events_;
    epoll_event worker_events_[NUM_WORKER][MAX_EVENTS];
    std::unique_ptr<Router> router_;

    void SetUpWorkerEpoll();
    void DistributeWork(int socket_fd);
    void ControlEpollEvent(int epoll_fd, int op, int fd, uint32_t events, void *data);
    void ProcessEpollEvents(int worker_id);
    void ProcessEpollInEvents(int epoll_fd, PeerState *state);
    void ProcessEpollOutEvents(int epoll_fd, PeerState *state);

    void HandleEpollEvent(int epoll_fd, PeerState *data,
                     std::uint32_t events);

    void control_epoll_event(int epoll_fd, int op, int fd,
                             std::uint32_t events = 0, void* data = nullptr);

    void HandleHttpData(const PeerState &raw_request,
                        PeerState *raw_response);
  };

 public:
  HttpServer(int num_worker, std::unique_ptr<Socket> &&socket);
  void Start(const std::string &host, int port);
  void Register(const std::string &path, HttpMethod method, std::function<HttpResponse(void)> handler);

 private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<ConnectionManager> cm_;
  int sock_fd_{0};
};

#endif//SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
