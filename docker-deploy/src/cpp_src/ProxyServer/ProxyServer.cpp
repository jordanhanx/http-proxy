/*------------------------------  ProxyServer.hpp  ---------------------------------*/
#include "ProxyServer.hpp"

ProxyServer::ProxyServer(const char * port, const char * threads) :
    thr_pool(std::stoi(threads) - 1),
    io_context_(std::stoi(threads)),
    acceptor_(
        io_context_,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::stoi(port))) {
  // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
  std::cout << "waiting for connections on " +
                   acceptor_.local_endpoint().address().to_string() + ":" +
                   std::to_string(acceptor_.local_endpoint().port()) + "\n";
}

void ProxyServer::do_accept() {
  acceptor_.async_accept(
      boost::asio::make_strand(io_context_),
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket client_socket) {
        if (!ec) {
          // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
          std::cout << "accepted connection from "
                    << client_socket.remote_endpoint().address().to_string() << ":"
                    << std::to_string(client_socket.remote_endpoint().port()) << "\n";
          std::make_shared<ProxySession>(std::move(client_socket), logger, cache)
              ->start();
        }
        else {
          std::cerr << "async_accept ec: " << ec.message() << "\n";
        }
        do_accept();
      });
}

void ProxyServer::runServer() {
  do_accept();
  // Multi threads call io_context::run() to invoke callback functions concurrently
  for (std::vector<std::thread>::size_type i = 0; i < thr_pool.size(); ++i) {
    thr_pool[i] = std::thread([this]() { io_context_.run(); });
  }
  io_context_.run();
}

/*------------------------------------  EOF  ---------------------------------------*/