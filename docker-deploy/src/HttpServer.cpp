/*------------------------------  HttpServer.cpp  ---------------------------------*/
#include "HttpServer.hpp"

// get sockaddr, IPv4 or IPv6:
void * HttpServer::get_in_addr(struct sockaddr * sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void HttpServer::shutDown() {
  freeaddrinfo(serverInfoList);
  close(serverSocket_fd);
}

void HttpServer::initServerInfo() {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;      // fill in my IP for me

  if (getaddrinfo(NULL, port, &hints, &serverInfoList) != 0) {
    std::stringstream ss;
    ss << "Error: cannot get address info for host (localhost," << port << ")";
    throw HttpServerError(ss.str());
  }
}

void HttpServer::initSocket() {
  // loop through all the results and bind to the first we can
  for (struct addrinfo * ptr = serverInfoList; ptr != NULL; ptr = ptr->ai_next) {
    if (tryBindSocket(ptr)) {
      return;
    }
  }
  std::stringstream ss;
  ss << "Error: cannot initialize socket for host (localhost," << port << ")";
  throw HttpServerError(ss.str());
}

bool HttpServer::tryBindSocket(const struct addrinfo * servInfo) {
  int32_t s_fd =
      socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
  if (s_fd == -1) {
    close(s_fd);
    return false;
  }
  int32_t yes = 1;
  if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    close(s_fd);
    return false;
  }
  if (bind(s_fd, servInfo->ai_addr, servInfo->ai_addrlen) == -1) {
    close(s_fd);
    return false;
  }
  serverSocket_fd = s_fd;
  return true;
}

void HttpServer::listenConn(int32_t backlog) {
  if (listen(serverSocket_fd, backlog) == -1) {
    std::stringstream ss;
    ss << "Error: cannot listen on socket for host (localhost," << port << ")";
    throw HttpServerError(ss.str());
  }
  else {
    out << "server: waiting for connections...\n";
  }
}

void HttpServer::acceptConn() {
  clientAddrSize = sizeof(clientAddr);
  clientSocket_fd =
      accept(serverSocket_fd, (struct sockaddr *)&clientAddr, &clientAddrSize);
  if (clientSocket_fd == -1) {
    std::stringstream ss;
    ss << "Error: cannot accept connection on socket for host (localhost," << port << ")";
    throw HttpServerError(ss.str());
  }
  else {
    char s[INET6_ADDRSTRLEN];
    inet_ntop(
        clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), s, sizeof(s));
    out << "server: got connection from " << s << "\n";
  }
}

void HttpServer::receive() {
  char recvBuffer[21] = {'\0'};
  int32_t status = recv(clientSocket_fd, recvBuffer, 20, 0);
  if (status == -1) {
    throw HttpServerError("Error: cannot recv()");
  }
  if (status == 0) {
    throw HttpServerError("remote connection closed.");
  }
  else {
    out << "" << recvBuffer;
  }
}

void HttpServer::respond() {
  // close(clientSocket_fd);
}

void HttpServer::launch(int32_t backlog) {
  try {
    listenConn(backlog);
    acceptConn();
    while (true) {
      receive();
      respond();
    }
  }
  catch (const HttpServerError & e) {
    std::cerr << e.what() << "\n";
  }
}

/*--------------------------------  EOF  ------------------------------------*/