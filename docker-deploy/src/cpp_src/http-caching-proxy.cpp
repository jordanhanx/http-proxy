#include <unistd.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "ProxyServer/ProxyServer.hpp"

int main(int argc, char ** argv) {
  try {
    // Run as a daemon
    if (daemon(0, 0) != 0) {
      throw std::runtime_error("Error: cannot become a daemon");
    }
    // default: single thread + asynchronous programming
    if (argc == 2) {
      ProxyServer server(argv[1], "1");
      server.runServer();
    }
    // thread pool's size is assigned by user
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