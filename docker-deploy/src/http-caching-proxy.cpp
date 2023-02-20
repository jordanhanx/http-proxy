#include <cstdlib>
#include <exception>
#include <iostream>

#include "AsynTcpServer/AsynTcpServer.hpp"

int main(int argc, char ** argv) {
  try {
    if (argc == 2) {
      AsynTcpServer server(argv[1]);
      server.runServer();
    }
    else {
      std::cerr << "Usage: ./http-caching-proxy <port>\n";
      exit(EXIT_FAILURE);
    }
  }
  catch (const std::exception & e) {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return EXIT_SUCCESS;
}