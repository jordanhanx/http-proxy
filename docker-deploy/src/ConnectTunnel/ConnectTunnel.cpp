/*------------------------------  ConnectTunnel.cpp  ---------------------------------*/
#include "ConnectTunnel.hpp"

ConnectTunnel::ConnectTunnel(boost::asio::ip::tcp::socket & client,
                             boost::asio::ip::tcp::socket & server) :
    client(client), server(server) {
}

ConnectTunnel::~ConnectTunnel() {
  std::cout << "Tunnel closed\n";
}

void ConnectTunnel::start(std::shared_ptr<void> session, const std::string & host) {
  keepSessionAlive = session;
  connectToServer(host);
}

void ConnectTunnel::connectToServer(const std::string & host) {
  boost::system::error_code ec;
  auto ipResolver = boost::asio::ip::tcp::resolver(server.get_executor());
  auto endpoints = ipResolver.resolve(host, "https", ec);
  if (ec) {
    std::cerr << "connectToServer(" << host << ":443) ec: " << ec.message() << "\n";
  }

  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  boost::asio::async_connect(server,
                             endpoints,
                             [this, self, host](boost::system::error_code ec,
                                                boost::asio::ip::tcp::endpoint ep) {
                               // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
                               if (!ec) {
                                 send200okToClient();
                               }
                               else {
                                 std::cerr << "connectToServer(" << host
                                           << ":443) ec: " << ec.message() << "\n";
                               }
                             });
}

void ConnectTunnel::send200okToClient() {
  boost::beast::http::response<boost::beast::http::empty_body> response = {};
  response.version(11);
  response.result(boost::beast::http::status::ok);

  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  boost::beast::http::async_write(
      client,
      response,
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          recvBytesFrOriginServer();
          recvBytesFrClient();
        }
        else {
          std::cerr << "send200okToClient() ec: " << ec.message() << "\n";
        }
      });
}

// upstream
void ConnectTunnel::recvBytesFrClient() {
  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  client.async_read_some(
      boost::asio::buffer(upstream_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          sendBytesToOriginServer(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrClient() ec: " << ec.message() << "\n";
        }
      });
}
void ConnectTunnel::sendBytesToOriginServer(std::size_t bytes_transferred) {
  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  boost::asio::async_write(
      server,
      boost::asio::buffer(upstream_buf, bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          recvBytesFrClient();
        }
        else {
          std::cerr << "sendBytesToOriginServer() ec: " << ec.message() << "\n";
        }
      });
}

// downstream
void ConnectTunnel::recvBytesFrOriginServer() {
  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  server.async_read_some(
      boost::asio::buffer(downstream_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          sendBytesToClient(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrOriginServer() ec: " << ec.message() << "\n";
        }
      });
}
void ConnectTunnel::sendBytesToClient(std::size_t bytes_transferred) {
  auto self = std::shared_ptr<ConnectTunnel>(keepSessionAlive, this);
  boost::asio::async_write(
      client,
      boost::asio::buffer(downstream_buf, bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          recvBytesFrOriginServer();
        }
        else {
          std::cerr << "sendBytesToClient() ec: " << ec.message() << "\n";
        }
      });
}

/*------------------------------------  EOF  ---------------------------------------*/