/*------------------------------  ProxySession.cpp  ---------------------------------*/
#include "ProxySession.hpp"

ProxySession::ProxySession(boost::asio::ip::tcp::socket socket) :
    client_socket(std::move(socket)), server_socket(socket.get_executor()) {
  client_address = client_socket.remote_endpoint().address().to_string() + ":" +
                   std::to_string(client_socket.remote_endpoint().port());
}

ProxySession::~ProxySession() {
  // client_socket.close();
  // server_socket.close();
  // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
  std::cout << "lost connection from client " << client_address << "\n";
}

void ProxySession::start() {
  std::cout << "readReqFrClient()...\n";
  recvReqFrClient();
}

void ProxySession::recvReqFrClient() {
  request = {};  // set request empty
  auto self(shared_from_this());
  boost::beast::http::async_read(
      client_socket,
      buf,
      request,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "readReqFrClient() successfully : \n[" << request << "]\n";
          auto suffix_pos = request.target().rfind(":443");
          auto host = std::string(request.target()).substr(0, suffix_pos);
          if (request.method() == boost::beast::http::verb::connect) {
            std::cout << "connectOriginServer(" << host << ":443)...\n";
            connectOriginServer(host, "443");
          }
          else if (request.method() == boost::beast::http::verb::get ||
                   request.method() == boost::beast::http::verb::post) {
            std::cout << "lookupCache()...\n";
            lookupCache(host, "80");
          }
          else {
            std::cerr << "readReqFrClient() unsupported method:\n[" << request << "]\n";
          }
        }
        else {
          std::cerr << "readReqFrClient() ec: " << ec.message() << "\n[" << request
                    << "]\n";
        }
      });
}

void ProxySession::sendReqToOriginServer() {
  auto self(shared_from_this());
  boost::beast::http::async_write(
      server_socket,
      request,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "sendReqToOriginServer() successfully\n";
          std::cout << "recvResFrOriginServer()...\n";
          recvResFrOriginServer();
        }
        else {
          std::cerr << "sendReqToOriginServer() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::recvResFrOriginServer() {
  response = {};  // set response empty
  auto self(shared_from_this());
  boost::beast::http::async_read(
      server_socket,
      buf,
      response,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "recvResFrOriginServer() successfully :\n[" << response.base()
                    << "]\n";
          std::cout << "updateCache()...\n";
          updateCache();
        }
        else {
          std::cerr << "recvResFrOriginServer() ec: " << ec.message() << "\n["
                    << response.base() << "]\n";
        }
      });
}

void ProxySession::sendResToClient() {
  auto self(shared_from_this());
  boost::beast::http::async_write(
      client_socket,
      response,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "sendResToClient() successfully\n";
        }
        else {
          std::cerr << "sendResToClient() ec: " << ec.message() << "\n";
        }
        // close session
      });
}

void ProxySession::connectOriginServer(const std::string & host,
                                       const std::string & port) {
  // Make the connection on the IP address we get from a lookup
  auto resolver_ = boost::asio::ip::tcp::resolver(server_socket.get_executor());
  boost::system::error_code ec;
  auto endpoints = resolver_.resolve(host, port, ec);
  if (ec) {
    std::cerr << "connectOriginServer(" << host << ":" << port << ") ec: " << ec.message()
              << "\n";
  }
  auto self(shared_from_this());
  boost::asio::async_connect(
      server_socket,
      endpoints,
      [this, self, host, port](boost::system::error_code ec,
                               boost::asio::ip::tcp::endpoint ep) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          server_address = ep.address().to_string() + ":" + std::to_string(ep.port());
          std::cout << "connectOriginServer(" << server_address << ") successfully\n";
          // if (port == "433")
          std::cout << "send200okToClient()...\n";
          send200okToClient();
          // else {
          //   sendReqToOriginServer();
          // }
        }
        else {
          std::cerr << "connectOriginServer(" << host << ":" << port
                    << ") ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::send200okToClient() {
  response = {};  // set response empty
  response.version(11);
  response.result(boost::beast::http::status::ok);
  auto self(shared_from_this());
  boost::beast::http::async_write(
      client_socket,
      response,
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "send200okToClient() sent " << bytes_transferred << " bytes \n["
                    << response << "]\n";
          std::cout << "recvBytesFrClient()...\n";
          recvBytesFrClient();
          std::cout << "recvBytesFrOriginServer()...\n";
          recvBytesFrOriginServer();
        }
        else {
          std::cerr << "send200okToClient() ec: " << ec.message() << "\n";
          std::cerr << response << "\n";
        }
      });
}

void ProxySession::recvBytesFrClient() {
  auto self(shared_from_this());
  client_socket.async_read_some(
      // boost::asio::buffer(bytes_buf),
      boost::asio::buffer(bytes_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "recvBytesFrClient(): successfully\n[";
          // std::cout.write((char *)bytes_buf.data(), bytes_transferred);
          std::cout << "]\n";
          std::cout << "sendBytesToOriginServer()...\n";
          sendBytesToOriginServer(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrClient() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::sendBytesToOriginServer(std::size_t bytes_transferred) {
  auto self(shared_from_this());
  boost::asio::async_write(
      server_socket,
      boost::asio::buffer(bytes_buf.data(), bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          std::cout << "sendBytesToOriginServer(): successfully\n";
          std::cout << "recvBytesFrClient()...\n";
          recvBytesFrClient();
        }
        else {
          std::cerr << "sendBytesToOriginServer() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::recvBytesFrOriginServer() {
  auto self(shared_from_this());
  server_socket.async_read_some(
      boost::asio::buffer(bytes_buf),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<server " << server_address << "> ";
        if (!ec) {
          std::cout << "recvBytesFrOriginServer(): successfully\n[";
          // std::cout.write((char *)bytes_buf.data(), bytes_transferred);
          std::cout << "]\n";
          std::cout << "sendBytesToClient()...\n";
          sendBytesToClient(bytes_transferred);
        }
        else {
          std::cerr << "recvBytesFrOriginServer() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::sendBytesToClient(std::size_t bytes_transferred) {
  auto self(shared_from_this());
  boost::asio::async_write(
      client_socket,
      boost::asio::buffer(bytes_buf, bytes_transferred),
      [this, self](boost::system::error_code ec, std::size_t /* bytes_transferred */) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<server " << server_address << "> ";
        if (!ec) {
          std::cout << "sendBytesToClient(): successfully\n";
          std::cout << "recvBytesFrOriginServer()...\n";
          recvBytesFrOriginServer();
        }
        else {
          std::cerr << "sendBytesToClient() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::lookupCache(const std::string & host, const std::string & port) {
  connectOriginServer(host, port);
}

void ProxySession::updateCache() {
  sendResToClient();
}
/*------------------------------------  EOF  ---------------------------------------*/