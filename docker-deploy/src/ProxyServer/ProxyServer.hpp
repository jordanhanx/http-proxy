/*------------------------------  ProxyServer.hpp  ---------------------------------*/
#ifndef __PROXY_SERVER_HPP__
#define __PROXY_SERVER_HPP__

#include <cstdlib>
#include <iostream>
#include <ostream>

#include "../Socket/Socket.hpp"

class ProxyServer {
 private:
  Socket socket;
  std::ostream & out;

 public:
  ProxyServer();
  explicit ProxyServer(const char * port);
  ProxyServer(const char * port, std::ostream & out);
  ~ProxyServer();

  void runServer();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/