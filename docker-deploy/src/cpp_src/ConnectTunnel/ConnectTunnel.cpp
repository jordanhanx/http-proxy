/*------------------------------  ConnectTunnel.cpp  ---------------------------------*/
#include "ConnectTunnel.hpp"

ConnectTunnel::ConnectTunnel(std::shared_ptr<void> session,
                             boost::asio::ip::tcp::socket & client,
                             boost::asio::ip::tcp::socket & server,
                             const std::string & request_id,
                             const std::string & server_host,
                             Logger & logger) :
    session_life_tracker(session),
    client(client),
    server(server),
    request_id(request_id),
    server_host(server_host),
    logger(logger) {
}

ConnectTunnel::~ConnectTunnel() {
  std::cout << "Tunnel closed\n";
  logger.log(request_id + ": Tunnel closed");
}

void ConnectTunnel::start() {
  connectToServer();
}

void ConnectTunnel::connectToServer() {
  boost::system::error_code ec;
  auto ipResolver = boost::asio::ip::tcp::resolver(server.get_executor());
  auto endpoints = ipResolver.resolve(server_host, "https", ec);
  if (ec) {
    std::cerr << "connectToServer(" << server_host << ":443) ec: " << ec.message()
              << "\n";
  }

  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  boost::asio::async_connect(
      server,
      endpoints,
      [this, self](boost::system::error_code ec, boost::asio::ip::tcp::endpoint ep) {
        if (!ec) {
          send200okToClient();
        }
        else {
          std::cerr << "connectToServer(" << server_host << ":443) ec: " << ec.message()
                    << "\n";
        }
      });
}

void ConnectTunnel::send200okToClient() {
  boost::beast::http::response<boost::beast::http::empty_body> response = {};
  response.version(11);
  response.result(boost::beast::http::status::ok);

  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  boost::beast::http::async_write(
      client,
      response,
      [this, self, response](boost::system::error_code ec,
                             std::size_t bytes_transferred) {
        if (!ec) {
          logger.logResponding(request_id,
                               response.version(),
                               response.result_int(),
                               response.reason().to_string());
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
  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  client.async_read_some(
      boost::asio::buffer(upstream_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          sendBytesToOriginServer(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrClient() ec: " << ec.message() << "\n";
        }
      });
}
void ConnectTunnel::sendBytesToOriginServer(std::size_t bytes_transferred) {
  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  boost::asio::async_write(
      server,
      boost::asio::buffer(upstream_buf, bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
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
  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  server.async_read_some(
      boost::asio::buffer(downstream_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          sendBytesToClient(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrOriginServer() ec: " << ec.message() << "\n";
        }
      });
}
void ConnectTunnel::sendBytesToClient(std::size_t bytes_transferred) {
  auto self = std::shared_ptr<ConnectTunnel>(session_life_tracker.lock(), this);
  boost::asio::async_write(
      client,
      boost::asio::buffer(downstream_buf, bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
        if (!ec) {
          recvBytesFrOriginServer();
        }
        else {
          std::cerr << "sendBytesToClient() ec: " << ec.message() << "\n";
        }
      });
}

/*------------------------------------  EOF  ---------------------------------------*/