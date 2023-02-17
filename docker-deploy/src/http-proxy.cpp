#include <cstdlib>
#include <iostream>

#include "HttpServer.hpp"

int main(int argc, char ** argv) {
  if (argc == 2) {
    HttpServer server(argv[1]);
    server.launch();
  }
  else if (argc == 1) {
    HttpServer server("12345");
    server.launch();
  }
  else {
    std::cerr << "Usage: ./http-proxy [port#]\n";
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}