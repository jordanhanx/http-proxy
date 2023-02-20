/*------------------------------  mThrTcpServer.hpp  ---------------------------------*/
#ifndef MTHR_TCP_SERVER_HPP
#define MTHR_TCP_SERVER_HPP

#include <boost/asio.hpp>

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

class mThrTcpServer {
 private:
  class Connection;

  std::vector<std::thread> thr_pool;
  /// The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;
  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  void do_accept();

 public:
  mThrTcpServer() = default;
  mThrTcpServer(const mThrTcpServer & rhs) = delete;
  mThrTcpServer & operator=(const mThrTcpServer & rhs) = delete;
  mThrTcpServer(const char * port, const char * threads);

  void runServer();
};

class mThrTcpServer::Connection : public std::enable_shared_from_this<Connection> {
 private:
  boost::asio::ip::tcp::socket socket_;
  std::string string_buffer;

  void readClientRequest();
  void sendClientResponse();

 public:
  Connection(boost::asio::ip::tcp::socket socket);
  ~Connection();
  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/