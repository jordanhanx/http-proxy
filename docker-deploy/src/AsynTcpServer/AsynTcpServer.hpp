/*------------------------------  AsynTcpServer.hpp  ---------------------------------*/
#ifndef ASYN_TCP_SERVER_HPP
#define ASYN_TCP_SERVER_HPP

#include <boost/asio.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

class AsynTcpServer {
 private:
  class Connection;
  /// The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;
  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  void do_accept();

 public:
  AsynTcpServer() = default;
  AsynTcpServer(const AsynTcpServer & rhs) = delete;
  AsynTcpServer & operator=(const AsynTcpServer & rhs) = delete;
  explicit AsynTcpServer(const char * port);

  void runServer();
};

class AsynTcpServer::Connection : public std::enable_shared_from_this<Connection> {
 private:
  boost::asio::ip::tcp::socket socket_;
  std::string string_buffer;

  void do_read();
  void do_write();

 public:
  Connection(boost::asio::ip::tcp::socket socket);
  ~Connection();

  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/