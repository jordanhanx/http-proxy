/*------------------------------  AsynTcpServer.cpp  ---------------------------------*/
#include "AsynTcpServer.hpp"

Connection::Connection(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {
}

void Connection::start() {
  do_read();
}

void Connection::do_read() {
  auto self(shared_from_this());
  boost::asio::async_read_until(
      socket_,
      boost::asio::dynamic_buffer(self->string_buffer),
      "\n",
      [self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          std::cout << self->string_buffer;
          self->do_write();
        }
      });
}

void Connection::do_write() {
  auto self(shared_from_this());
  boost::asio::async_write(socket_,
                           boost::asio::buffer("echo: " + self->string_buffer),
                           [self](boost::system::error_code ec, std::size_t /*length*/) {
                             if (!ec) {
                               self->string_buffer.clear();
                               self->do_read();
                             }
                           });
}

AsynTcpServer::AsynTcpServer(const char * port) :
    io_context_(1),
    acceptor_(
        io_context_,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::atoi(port))) {
  std::cout << "server: waiting for connections on " +
                   acceptor_.local_endpoint().address().to_string() + ":" +
                   std::to_string(acceptor_.local_endpoint().port()) + "\n";
}

void AsynTcpServer::do_accept() {
  acceptor_.async_accept(
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
          std::cout << "server: got connection from " +
                           socket.remote_endpoint().address().to_string() + ":" +
                           std::to_string(socket.remote_endpoint().port()) + "\n";
          std::make_shared<Connection>(std::move(socket))->start();
        }
        do_accept();
      });
}

void AsynTcpServer::runServer() {
  do_accept();
  io_context_.run();
  std::cout << "server closed.\n";
}

/*------------------------------------  EOF  ---------------------------------------*/