#include "api/greeting.h"
#include "api/index.h"
#include <iostream>
#include <memory>
#include <networking/http_server.h>

int main() {
  {
    int num_worker = 5;
    const std::string host = "0.0.0.0";
    const int port = 8080;
    auto socket = std::make_unique<Socket>();
    HttpServer server{num_worker, std::move(socket)};

    server.Register("/", HttpMethod::GET, Greeting);
    server.Register("/index.html", HttpMethod::GET, RenderIndexHtmL);

    try {
      server.Start(host, port);
      std::cout << "Press `q` or `quit` to stop" << std::endl;
      std::string cmd;
      while (true) {
        std::cin >> cmd;
        if (cmd == "q" || cmd == "quit") {
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    } catch (std::exception& e) {
      std::cout << "Something went wrong: " << e.what() << std::endl;
    }

  }

  std::cout << "The server has stopped" << std::endl;
  return 0;
}
