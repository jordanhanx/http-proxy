/*------------------------------  ProxyServer.hpp  ---------------------------------*/
#ifndef PROXY_SERVER_HPP
#define PROXY_SERVER_HPP

#include <boost/asio.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "../Logger/Logger.hpp"
#include "../ProxySession/ProxySession.hpp"

class ProxyServer {
 private:
  /// threads pool
  std::vector<std::thread> thr_pool;
  /// The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;
  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;
  /// logger
  Logger logger{"./proxy.log"};

  void do_accept();

 public:
  ProxyServer() = default;
  ProxyServer(const ProxyServer & rhs) = delete;
  ProxyServer & operator=(const ProxyServer & rhs) = delete;
  ProxyServer(const char * port, const char * threads);

  void runServer();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/