#include <cstdlib>
#include <iostream>

#include "proxy-server/ProxyServer.hpp"

int main(int argc, char ** argv) {
  if (argc == 2) {
    ProxyServer server(argv[1]);
    server.runServer();
  }
  else if (argc == 1) {
    ProxyServer server("12345");
    server.runServer();
  }
  else {
    std::cerr << "Usage: ./http-caching-proxy [port, default=12345]\n";
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}