/*------------------------------  Socket.hpp  ---------------------------------*/
#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

class Socket {
 private:
  const char * hostname;
  const char * port;
  struct addrinfo * hostInfoList;
  struct addrinfo * hostInfo;
  int32_t socket_fd;
  int32_t listen_fd;
  struct sockaddr_storage listenAddr;
  socklen_t listenAddrLen;

  void initHostInfoList();
  void initSocketfd();

  static void * getInetAddr(struct sockaddr * sa);

 public:
  Socket();
  explicit Socket(const char * port);
  Socket(const char * hostname, const char * port);
  ~Socket();

  void connectHost();
  void listenOnPort(int32_t backlog = 10);
  void acceptConn();
  size_t sendAll(const std::string & msg) const;
  std::string recvLine() const;

  std::string getSocketIP() const;
  std::string getListenIP() const;
};

class SocketError : public std::runtime_error {
 public:
  explicit SocketError(const std::string & what_arg) : runtime_error(what_arg) {}
};

#endif
/*----------------------------------  EOF  ------------------------------------*/