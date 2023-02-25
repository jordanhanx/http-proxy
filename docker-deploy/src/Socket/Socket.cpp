/*------------------------------  Socket.cpp  -------------------------------*/
#include "Socket.hpp"

Socket::Socket() : hostname(NULL), port(NULL) {
  initHostInfoList();
  initSocketfd();
  listenAddrLen = sizeof(listenAddr);
}

Socket::Socket(const char * port) : hostname(NULL), port(port) {
  initHostInfoList();
  initSocketfd();
  listenAddrLen = sizeof(listenAddr);
}

Socket::Socket(const char * hostname, const char * port) :
    hostname(hostname), port(port) {
  initHostInfoList();
  initSocketfd();
  listenAddrLen = sizeof(listenAddr);
}

Socket::~Socket() {
  freeaddrinfo(hostInfoList);
  close(socket_fd);
}

void Socket::initHostInfoList() {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));  // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;       // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
  if (hostname == NULL) {
    hints.ai_flags = AI_PASSIVE;  // fill in my IP for me
  }

  if (getaddrinfo(hostname, port, &hints, &hostInfoList) != 0) {
    std::stringstream ss;
    ss << "Error: cannot get address info for host" << hostname << ", " << port << ")";
    throw SocketError(ss.str());
  }
}

void Socket::initSocketfd() {
  // loop through all the results and bind to the first we can
  for (struct addrinfo * info = hostInfoList; info != NULL; info = info->ai_next) {
    socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (socket_fd != -1) {
      hostInfo = info;
      return;
    }
  }
  std::stringstream ss;
  ss << "Error: cannot create socket for host" << hostname << ", " << port << ")";
  throw SocketError(ss.str());
}

void Socket::connectHost() {
  if (connect(socket_fd, hostInfo->ai_addr, hostInfo->ai_addrlen) == -1) {
    std::stringstream ss;
    ss << "Error: cannot connect to socket for host" << hostname << ", " << port << ")";
    throw SocketError(ss.str());
  }
}

void Socket::listenOnPort(int32_t backlog) {
  int32_t yes = 1;  // lose the pesky "Address already in use" error message
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (bind(socket_fd, hostInfo->ai_addr, hostInfo->ai_addrlen) == -1) {
    std::stringstream ss;
    ss << "Error: cannot bind socket for host" << hostname << ", " << port << ")";
    throw SocketError(ss.str());
  }
  if (listen(socket_fd, backlog) == -1) {
    std::stringstream ss;
    ss << "Error: cannot listen on socket for host" << hostname << ", " << port << ")";
    throw SocketError(ss.str());
  }
}

void Socket::acceptConn() {
  listen_fd = accept(socket_fd, (struct sockaddr *)&listenAddr, &listenAddrLen);
  if (listen_fd == -1) {
    std::stringstream ss;
    ss << "Error: cannot accept connection on socket for host (localhost," << port << ")";
    throw SocketError(ss.str());
  }
}

size_t Socket::sendAll(const std::string & msg) const {
  const char * buf = msg.c_str();
  size_t total = 0;                 // how many bytes we've sent
  size_t bytesleft = msg.length();  // how many we have left to send
  ssize_t n;
  while (total < msg.length()) {
    n = send(socket_fd, buf + total, bytesleft, 0);
    if (n == -1) {
      std::stringstream ss;
      ss << "Error: cannot send message to socket for host" << hostname << ", " << port
         << ")";
      throw SocketError(ss.str());
    }
    total += n;
    bytesleft -= n;
  }
  return total;  // return number actually sent bytes
}

std::string Socket::recvLine() const {
  std::stringstream ss;
  char buf[1] = {'\0'};
  while (buf[0] != '\n') {
    int32_t status = recv(listen_fd, buf, 1, 0);
    if (status == -1) {
      throw SocketError("Error: cannot recv()");
    }
    if (status == 0) {
      throw SocketError("Error: remote connection closed.");
    }
    ss << buf[0];
  }
  return ss.str();
}

// get sockaddr, IPv4 or IPv6:
void * Socket::getInetAddr(struct sockaddr * sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

std::string Socket::getSocketIP() const {
  char buf[INET6_ADDRSTRLEN];
  if (inet_ntop(
          hostInfo->ai_family, getInetAddr(hostInfo->ai_addr), buf, INET6_ADDRSTRLEN) ==
      NULL) {
    throw SocketError("Error: cannot convert addresses from binary to text form");
  }
  std::stringstream ss;
  ss << buf << ":" << port;
  return ss.str();
}

std::string Socket::getListenIP() const {
  char buf[INET6_ADDRSTRLEN];
  if (inet_ntop(listenAddr.ss_family,
                getInetAddr((struct sockaddr *)&listenAddr),
                buf,
                INET6_ADDRSTRLEN) == NULL) {
    throw SocketError("Error: cannot convert addresses from binary to text form");
  }
  std::stringstream ss;
  ss << buf << ":" << ((struct sockaddr_in *)&listenAddr)->sin_port;
  return ss.str();
}

/*----------------------------------  EOF  ------------------------------------*/