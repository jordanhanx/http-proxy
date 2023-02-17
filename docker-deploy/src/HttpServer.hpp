/*------------------------------  HttpServer.hpp  ---------------------------------*/
#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class HttpServer {
 private:
  int32_t serverSocket_fd;
  int32_t clientSocket_fd;
  const char * port;
  std::ostream & out;
  struct addrinfo * serverInfoList;
  struct sockaddr_storage clientAddr;
  socklen_t clientAddrSize;

  static void * get_in_addr(struct sockaddr * sa);

  void shutDown();
  void initServerInfo();
  void initSocket();
  bool tryBindSocket(const struct addrinfo * servInfo);
  void listenConn(int32_t backlog);
  void acceptConn();
  void receive();
  void respond();

 public:
  HttpServer() : port("12345"), out(std::cout) {
    initServerInfo();
    initSocket();
  }
  explicit HttpServer(const char * port) : port(port), out(std::cout) {
    initServerInfo();
    initSocket();
  }
  HttpServer(const char * port, std::ostream & out) : port(port), out(out) {
    initServerInfo();
    initSocket();
  }
  ~HttpServer() { shutDown(); }

  void launch(int32_t backlog = 10);
};

class HttpServerError : public std::runtime_error {
 public:
  explicit HttpServerError(const std::string & what_arg) : runtime_error(what_arg) {}
};

#endif
/*--------------------------------  EOF  ------------------------------------*/