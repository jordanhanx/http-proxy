/*------------------------------  SimpleTcpServer.hpp  ---------------------------------*/
#ifndef SIMPLE_TCP_SERVER_HPP
#define SIMPLE_TCP_SERVER_HPP

#include <cstdlib>
#include <iostream>
#include <ostream>

#include "../Socket/Socket.hpp"

class SimpleTcpServer {
 private:
  Socket socket;
  std::ostream & out;

 public:
  SimpleTcpServer() = default;
  SimpleTcpServer(const SimpleTcpServer & rhs) = delete;
  SimpleTcpServer & operator=(const SimpleTcpServer & rhs) = delete;
  explicit SimpleTcpServer(const char * port);

  void runServer();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/