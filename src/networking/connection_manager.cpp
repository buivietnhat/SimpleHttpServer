
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

}

void HttpServer::ConnectionManager::SetUpWorkerEpoll() {
  for (int i = 0; i < num_workers_; i++) {
    auto epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
      throw std::runtime_error("failed to create worker's file descriptor");
    }
    worker_epoll_fd_[i] = epoll_fd;

//    auto *worker_event = new epoll_event[MAX_EVENTS];
//    worker_events_.push_back(worker_event);
  }
}

void HttpServer::ConnectionManager::DistributeWork(int socket_fd) {
  std::cout << "distribute work running" << std::endl;
  int accepted_socket_fd{-1};
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);


  while (!killed_) {
    accepted_socket_fd = accept4(socket_fd, (sockaddr *) &addr, &addr_len,
                                 SOCK_NONBLOCK);
    if (accepted_socket_fd < 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      continue;
    }

    cout << "new connection has came with fd " << accepted_socket_fd << endl;
    auto peer_state = new PeerState();
    peer_state->fd = accepted_socket_fd;
    control_epoll_event(worker_epoll_fd_[current_worker_idx], EPOLL_CTL_ADD, accepted_socket_fd,
                      EPOLLIN, peer_state);

    // round-robin manner
    current_worker_idx = (current_worker_idx + 1) % NUM_WORKER;
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
  SetUpWorkerEpoll();

  controll_thread_ = std::thread(&HttpServer::ConnectionManager::DistributeWork, this, socket_fd);

  for (int worker_idx = 0; worker_idx < num_workers_; worker_idx++) {
    workers_[worker_idx] = std::thread(&HttpServer::ConnectionManager::ProcessEpollEvents, this, worker_idx);
  }
}



void HttpServer::ConnectionManager::ProcessEpollEvents(int worker_id) {
  std::cout << "ProcessEpollEvents running for worker id " << worker_id << std::endl;
  auto epoll_fd = worker_epoll_fd_[worker_id];

  while (!killed_) {
    auto num_fd = epoll_wait(worker_epoll_fd_[worker_id], worker_events_[worker_id],
                             HttpServer::ConnectionManager::MAX_EVENTS, 0);
    if (num_fd <= 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
      continue;
    }

    for (int i = 0; i < num_fd; i++) {
      const epoll_event &event = worker_events_[worker_id][i];
      auto *peer_state = reinterpret_cast<PeerState *>(event.data.ptr);

      if ((event.events & EPOLLHUP) ||
          (event.events & EPOLLERR)) {
        control_epoll_event(epoll_fd, EPOLL_CTL_DEL, peer_state->fd);
        close(peer_state->fd);
        delete peer_state;
      } else if ((event.events == EPOLLIN) ||
                 (event.events == EPOLLOUT)) {
        HandleEpollEvent(epoll_fd, peer_state, event.events);
      } else {  // something unexpected
        control_epoll_event(epoll_fd, EPOLL_CTL_DEL, peer_state->fd);
        close(peer_state->fd);
        delete peer_state;
      }

//      if ((event.events & EPOLLIN)) {
//        ProcessEpollInEvents(epoll_fd, peer_state);
//        continue;
//      }
//
//      if ((event.events & EPOLLOUT)) {
//        ProcessEpollOutEvents(epoll_fd, peer_state);
//        continue;
//      }
//
//      // something went wrong
//      ControlEpollEvent(epoll_fd, EPOLL_CTL_DEL, peer_state->fd, 0, nullptr);
//      close(peer_state->fd);
//      if (peer_state) {
//        delete peer_state;
//      }
    }
  }
}

HttpServer::ConnectionManager::~ConnectionManager() {
  cout << "Destructor got called" << endl;
  killed_ = true;
  controll_thread_.join();
  for (int worker_idx = 0; worker_idx < num_workers_; worker_idx++) {
    workers_[worker_idx].join();
    // free all the worker_events resources
//    delete[] worker_events_[worker_idx];

    // close workers_ file descritor
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
      auto *new_state = new PeerState();
      new_state->fd = state->fd;
      ControlEpollEvent(epoll_fd, EPOLL_CTL_MOD, state->fd, EPOLLIN, new_state);
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
//  memset(buffer, 0, BUFFER_SIZE);
}

void HttpServer::ConnectionManager::HandleEpollEvent(int epoll_fd, HttpServer::PeerState *data, std::uint32_t events) {
  int fd = data->fd;
  PeerState *request, *response;

  if (events == EPOLLIN) {
    request = data;
    ssize_t byte_count = recv(fd, request->buffer, BUFFER_SIZE, 0);
    if (byte_count > 0) {  // we have fully received the message
      response = new PeerState();
      response->fd = fd;
      HandleHttpData(*request, response);
      control_epoll_event(epoll_fd, EPOLL_CTL_MOD, fd, EPOLLOUT, response);
      delete request;
    } else if (byte_count == 0) {  // client has closed connection
      control_epoll_event(epoll_fd, EPOLL_CTL_DEL, fd);
      close(fd);
      delete request;
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {  // retry
        request->fd = fd;
        control_epoll_event(epoll_fd, EPOLL_CTL_MOD, fd, EPOLLIN, request);
      } else {  // other error
        control_epoll_event(epoll_fd, EPOLL_CTL_DEL, fd);
        close(fd);
        delete request;
      }
    }
  } else {
    response = data;
    ssize_t byte_count =
        send(fd, response->buffer + response->sendptr, response->length, 0);
    if (byte_count >= 0) {
      if (byte_count < response->length) {  // there are still bytes to write
        response->sendptr += byte_count;
        response->length -= byte_count;
        control_epoll_event(epoll_fd, EPOLL_CTL_MOD, fd, EPOLLOUT, response);
      } else {  // we have written the complete message
        request = new PeerState();
        request->fd = fd;
        control_epoll_event(epoll_fd, EPOLL_CTL_MOD, fd, EPOLLIN, request);
        delete response;
      }
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {  // retry
        control_epoll_event(epoll_fd, EPOLL_CTL_ADD, fd, EPOLLOUT, response);
      } else {  // other error
        control_epoll_event(epoll_fd, EPOLL_CTL_DEL, fd);
        close(fd);
        delete response;
      }
    }
  }
}
void HttpServer::ConnectionManager::control_epoll_event(int epoll_fd, int op, int fd, std::uint32_t events, void *data) {
  if (op == EPOLL_CTL_DEL) {
    if (epoll_ctl(epoll_fd, op, fd, nullptr) < 0) {
      throw std::runtime_error("Failed to remove file descriptor");
    }
  } else {
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = data;
    if (epoll_ctl(epoll_fd, op, fd, &ev) < 0) {
      throw std::runtime_error("Failed to add file descriptor");
    }
  }
}
void HttpServer::ConnectionManager::HandleHttpData(const HttpServer::PeerState &raw_request, HttpServer::PeerState *raw_response) {
  std::string request_string(raw_request.buffer), response_string;
//  HttpRequest http_request;
  HttpResponse http_response;

//  try {
//    http_request = string_to_request(request_string);
//    http_response = HandleHttpRequest(http_request);
//  } catch (const std::invalid_argument &e) {
//    http_response = HttpResponse(HttpStatusCode::BadRequest);
//    http_response.SetContent(e.what());
//  } catch (const std::logic_error &e) {
//    http_response = HttpResponse(HttpStatusCode::HttpVersionNotSupported);
//    http_response.SetContent(e.what());
//  } catch (const std::exception &e) {
//    http_response = HttpResponse(HttpStatusCode::InternalServerError);
//    http_response.SetContent(e.what());
//  }
//
//  // Set response to write to client
//  response_string =
//      to_string(http_response, http_request.method() != HttpMethod::HEAD);
  response_string = "HTTP/1.1 200 OK\r\n"
                                "Content-Length: 13\r\n"
                                "Content-Type: text/plain\r\n\r\n"
                                "Hello, world\n";
  std::cout << response_string << std::endl;
  memcpy(raw_response->buffer, response_string.c_str(), BUFFER_SIZE);
  raw_response->length = response_string.length();
}
