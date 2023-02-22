/*------------------------------  mThrTcpServer.hpp  ---------------------------------*/
#include "mThrTcpServer.hpp"

mThrTcpServer::mThrTcpServer(const char * port, const char * threads) :
    thr_pool(std::stoi(threads) - 1),
    io_context_(std::stoi(threads)),
    acceptor_(
        io_context_,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::stoi(port))) {
  std::cout << "server (thr_id=" << std::this_thread::get_id()
            << "): waiting for connections on " +
                   acceptor_.local_endpoint().address().to_string() + ":" +
                   std::to_string(acceptor_.local_endpoint().port()) + "\n";
}

void mThrTcpServer::do_accept() {
  acceptor_.async_accept(
      boost::asio::make_strand(io_context_),
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
          std::cout << "server (thr_id=" << std::this_thread::get_id()
                    << "): got connection from "
                    << socket.remote_endpoint().address().to_string() << ":"
                    << std::to_string(socket.remote_endpoint().port()) << "\n";
          std::make_shared<Connection>(std::move(socket))->start();
        }
        do_accept();
      });
}

void mThrTcpServer::runServer() {
  do_accept();
  for (std::vector<std::thread>::size_type i = 0; i < thr_pool.size(); ++i) {
    thr_pool[i] = std::thread([this]() { io_context_.run(); });
  }
  io_context_.run();
  std::cout << "server closed.\n";
}

mThrTcpServer::Connection::Connection(boost::asio::ip::tcp::socket socket) :
    socket_(std::move(socket)) {
}

mThrTcpServer::Connection::~Connection() {
  std::cout << "server (thr_id=" << std::this_thread::get_id()
            << "): connection close from "
            << socket_.remote_endpoint().address().to_string() << ":"
            << std::to_string(socket_.remote_endpoint().port()) << "\n";
}

void mThrTcpServer::Connection::start() {
  readClientRequest();
}

void mThrTcpServer::Connection::readClientRequest() {
  auto self(shared_from_this());
  boost::asio::async_read_until(
      socket_,
      boost::asio::dynamic_buffer(self->string_buffer),
      "\n",
      [self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          std::cout << "server (thr_id=" << std::this_thread::get_id()
                    << ") received:" << self->string_buffer;
          self->sendClientResponse();
        }
      });
}

void mThrTcpServer::Connection::sendClientResponse() {
  auto self(shared_from_this());
  boost::asio::async_write(socket_,
                           boost::asio::buffer("echo: " + self->string_buffer),
                           [self](boost::system::error_code ec, std::size_t /*length*/) {
                             if (!ec) {
                               self->string_buffer.clear();
                               self->readClientRequest();
                             }
                           });
}

/*------------------------------------  EOF  ---------------------------------------*/