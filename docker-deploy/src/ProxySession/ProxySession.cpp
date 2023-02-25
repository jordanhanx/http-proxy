/*------------------------------  ProxySession.cpp  ---------------------------------*/
#include "ProxySession.hpp"

ProxySession::ProxySession(boost::asio::ip::tcp::socket socket) :
    client(std::move(socket)), server(socket.get_executor()) {
  client_address = client.remote_endpoint().address().to_string() + ":" +
                   std::to_string(client.remote_endpoint().port());
}

ProxySession::~ProxySession() {
}

void ProxySession::start() {
  // std::cout << "readReqFrClient()...\n";
  recvReqFrClient();
}

void ProxySession::recvReqFrClient() {
  auto self = shared_from_this();
  request = {};
  boost::beast::http::async_read(
      client,
      buf,
      request,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          auto suffix_pos = request[boost::beast::http::field::host].rfind(":443");
          auto host =
              std::string(request[boost::beast::http::field::host]).substr(0, suffix_pos);
          if (request.method() == boost::beast::http::verb::connect) {
            tunnel = std::unique_ptr<ConnectTunnel>(new ConnectTunnel(client, server));
            tunnel->start(self, host);
          }
          else if (request.method() == boost::beast::http::verb::post) {
            sendReqToOriginServer();
          }
          else if (request.method() == boost::beast::http::verb::get) {
            lookupCache();
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
  auto self = shared_from_this();
  boost::beast::http::async_write(
      server,
      request,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          recvResFrOriginServer();
        }
        else {
          std::cerr << "sendReqToOriginServer() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::recvResFrOriginServer() {
  auto self = shared_from_this();
  response = {};
  boost::beast::http::async_read(
      server,
      buf,
      response,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          // std::cout << "recvResFrOriginServer() successfully :\n[" << response.base()
          //           << "]\n";
          // std::cout << "updateCache()...\n";
          updateCache();
        }
        else {
          std::cerr << "recvResFrOriginServer() ec: " << ec.message() << "\n["
                    << response.base() << "]\n";
        }
      });
}

void ProxySession::sendResToClient() {
  auto self = shared_from_this();
  boost::beast::http::async_write(
      client,
      response,
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        std::cout << "<client " << client_address << "> ";
        if (!ec) {
          recvReqFrClient();
        }
        else {
          std::cerr << "sendResToClient() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::connectOriginServer(const std::string & host) {
  boost::system::error_code ec;
  // Make the connection on the IP address we get from a lookup
  auto ipResolver = boost::asio::ip::tcp::resolver(server.get_executor());
  auto endpoints = ipResolver.resolve(host, "http", ec);
  if (ec) {
    std::cerr << "connectOriginServer(" << host << ":80) ec: " << ec.message() << "\n";
  }
  auto self = shared_from_this();
  boost::asio::async_connect(server,
                             endpoints,
                             [this, self, host](boost::system::error_code ec,
                                                boost::asio::ip::tcp::endpoint ep) {
                               // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
                               if (!ec) {
                                 server_address = ep.address().to_string() + ":" +
                                                  std::to_string(ep.port());
                                 sendReqToOriginServer();
                               }
                               else {
                                 std::cerr << "connectOriginServer(" << host
                                           << ":80) ec: " << ec.message() << "\n";
                               }
                             });
}

void ProxySession::lookupCache() {
  auto suffix_pos = request[boost::beast::http::field::host].rfind(":443");
  auto host = std::string(request[boost::beast::http::field::host]).substr(0, suffix_pos);
  connectOriginServer(host);
}

void ProxySession::updateCache() {
  sendResToClient();
}
/*------------------------------------  EOF  ---------------------------------------*/