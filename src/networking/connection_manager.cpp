
#include "include/networking/http_server.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

using std::cout;
using std::endl;

// one worker for listening and distributing work
HttpServer::ConnectionManager::ConnectionManager(int num_workers, std::unique_ptr<Router> &&router)
    : num_workers_(num_workers - 1), router_(std::move(router)) {
  SetUpWorkerEpoll();
}

void HttpServer::ConnectionManager::SetUpWorkerEpoll() {
  for (int i = 0; i < num_workers_; i++) {
    auto epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
      throw std::runtime_error("failed to create worker's file descriptor");
    }
    worker_epoll_fd_.push_back(epoll_fd);

    auto *worker_event = new epoll_event[MAX_EVENTS];
    worker_events_.push_back(worker_event);
  }
}

void HttpServer::ConnectionManager::DistributeWork(int socket_fd) {
  std::cout << "distribute work running" << std::endl;
  int accepted_socket_fd{-1};
  sockaddr_in addr;
  socklen_t addr_len;

  while (!killed_) {
    accepted_socket_fd = accept4(socket_fd, (sockaddr *) &addr, &addr_len,
                                 SOCK_NONBLOCK);
    if (accepted_socket_fd == -1) {
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      continue;
    }

    cout << "new connection has came with fd " << accepted_socket_fd << endl;
    auto peer_state = new PeerState();
    peer_state->fd = accepted_socket_fd;
    ControlEpollEvent(worker_epoll_fd_[current_worker_idx], EPOLL_CTL_ADD, accepted_socket_fd,
                      EPOLLIN, peer_state);

    // round-robin manner
    current_worker_idx = (current_worker_idx + 1) % num_workers_;
  }
}

void HttpServer::ConnectionManager::ControlEpollEvent(int epoll_fd, int op, int fd, uint32_t events,
                                                      void *peer_state) {
  if (op == EPOLL_CTL_DEL) {
    if (epoll_ctl(epoll_fd, op, fd, nullptr) < 0) {
      throw std::runtime_error("failed to remove file descriptor");
    }
    return;
  }

  epoll_event ev;
  ev.events = events;
  ev.data.ptr = peer_state;

  auto res = epoll_ctl(epoll_fd, op, fd, &ev);
  if (res < 0) {
    throw std::runtime_error("failed to add file descriptor");
  }
}

void HttpServer::ConnectionManager::ListenAndProcess(int socket_fd) {
  controll_thread_ = std::thread(&HttpServer::ConnectionManager::DistributeWork, this, socket_fd);

  for (int worker_idx = 0; worker_idx < num_workers_; worker_idx++) {
    workers.push_back(std::thread(&HttpServer::ConnectionManager::ProcessEpollEvents, this, worker_idx));
  }
}

void HttpServer::ConnectionManager::ProcessEpollEvents(int worker_id) {
  std::cout << "ProcessEpollEvents running for worker id " << worker_id << std::endl;
  auto epoll_fd = worker_epoll_fd_[worker_id];

  while (!killed_) {
    auto num_fd = epoll_wait(worker_epoll_fd_[worker_id], worker_events_[worker_id],
                             HttpServer::ConnectionManager::MAX_EVENTS, 0);
    if (num_fd <= 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      continue;
    }

    for (int i = 0; i < num_fd; i++) {
      const auto &event = worker_events_[worker_id][i];
      auto *peer_state = reinterpret_cast<PeerState *>(event.data.ptr);

      if ((event.events & EPOLLIN)) {
        ProcessEpollInEvents(epoll_fd, peer_state);
        continue;
      }

      if ((event.events & EPOLLOUT)) {
        ProcessEpollOutEvents(epoll_fd, peer_state);
        continue;
      }

      // something went wrong
      ControlEpollEvent(epoll_fd, EPOLL_CTL_DEL, peer_state->fd, 0, nullptr);
      close(peer_state->fd);
      if (peer_state) {
        delete peer_state;
      }
    }
  }
}

HttpServer::ConnectionManager::~ConnectionManager() {
  cout << "Destructor got called" << endl;
  killed_ = true;
  controll_thread_.join();
  for (int worker_idx = 0; worker_idx < num_workers_; worker_idx++) {
    workers[worker_idx].join();
    // free all the worker_events resources
    delete[] worker_events_[worker_idx];

    // close workers file descritor
    close(worker_epoll_fd_[worker_idx]);
  }
}

void HttpServer::ConnectionManager::ProcessEpollInEvents(int epoll_fd, PeerState *state) {
  auto recived_size = recv(state->fd, state->buffer, BUFFER_SIZE, 0);
  if (recived_size > 0) {// the message has came
    HttpResponse http_response;
    bool content_included =  true;

    try {
      auto http_request = MessageParser::ToHttpRequest(std::string(state->buffer));
      if (http_request.GetStartLine().method_ == HttpMethod::HEAD) {
        content_included = false;
      }
      auto path = http_request.GetStartLine().request_target_;
      http_response = router_->Serve(path, http_request);
    } catch (const std::invalid_argument &e) {
      http_response.SetStatusCode(HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED);
    } catch (const std::logic_error &e) {
      http_response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    } catch(const std::exception &e) {
      http_response.SetStatusCode(HttpStatusCode::INTERNAL_SERVER_ERROR);
    }

    auto *peer_state_respone = MessageParser::ToPeerState(state->fd, http_response, content_included);
    ControlEpollEvent(epoll_fd, EPOLL_CTL_MOD, state->fd, EPOLLOUT, peer_state_respone);
    delete state;

  } else if (recived_size == 0) {// the connection has been closed
    ControlEpollEvent(epoll_fd, EPOLL_CTL_DEL, state->fd, 0, nullptr);
    close(state->fd);
    if (state) {
      delete state;
    }
  } else {                                        // something went wrong
    if (errno == EAGAIN || errno == EWOULDBLOCK) {// retry
      ControlEpollEvent(epoll_fd, EPOLL_CTL_MOD, state->fd, EPOLLIN, state);
    } else {// other error
      ControlEpollEvent(epoll_fd, EPOLL_CTL_DEL, state->fd, 0, nullptr);
      close(state->fd);
      if (state) {
        delete state;
      }
    }
  }
}

void HttpServer::ConnectionManager::ProcessEpollOutEvents(int epoll_fd, PeerState *state) {
  auto bytes_has_sent = send(state->fd, &state->buffer[state->sendptr], state->length, 0);
  if (bytes_has_sent >= 0) {
    if (bytes_has_sent < state->length) {// still bytes remain to send
      state->sendptr += bytes_has_sent;
      state->length -= bytes_has_sent;
      ControlEpollEvent(epoll_fd, EPOLL_CTL_MOD, state->fd, EPOLLOUT, state);
    } else {// all the message has been sent
      close(state->fd);
      delete state;
    }
  } else {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {// retry
      ControlEpollEvent(epoll_fd, EPOLL_CTL_ADD, state->fd, EPOLLOUT, state);
    } else {// other error
      ControlEpollEvent(epoll_fd, EPOLL_CTL_DEL, state->fd, 0, nullptr);
      close(state->fd);
      delete state;
    }
  }
}

void HttpServer::ConnectionManager::Register(const std::string &path, HttpMethod method,
                                             std::function<HttpResponse(void)> handler) {
  router_->Register(path, method, handler);
}

HttpServer::PeerState::PeerState() {
  fd = 0;
  sendptr = 0;
  length = 0;
  memset(buffer, 0, BUFFER_SIZE);
}
