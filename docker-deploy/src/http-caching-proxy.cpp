#include <cstdlib>
#include <exception>
#include <iostream>

// #include "mThrTcpServer/mThrTcpServer.hpp"
#include "ProxyServer/ProxyServer.hpp"

int main(int argc, char ** argv) {
  try {
    if (argc == 2) {
      ProxyServer server(argv[1], "1");
      server.runServer();
    }
    else if (argc == 3) {
      ProxyServer server(argv[1], argv[2]);
      server.runServer();
    }
    else {
      std::cerr << "Invalid arguments!\n"
                << "Usage: ./http-caching-proxy <port> [<threads>]\n";
      exit(EXIT_FAILURE);
    }
  }
  catch (const std::exception & e) {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return EXIT_SUCCESS;
}