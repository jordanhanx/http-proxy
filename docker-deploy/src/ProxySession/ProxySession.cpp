/*------------------------------  ProxySession.cpp  ---------------------------------*/
#include "ProxySession.hpp"

std::atomic<size_t> ProxySession::nextRequestID(1);

ProxySession::ProxySession(boost::asio::ip::tcp::socket socket, Logger & logger) :
    client(std::move(socket)), server(socket.get_executor()), logger(logger) {
  client_ip = client.remote_endpoint().address().to_string();
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
        request.set("requestID", nextRequestID);
        nextRequestID++;
        if (!ec) {
          auto suffix_pos = request[boost::beast::http::field::host].rfind(":443");
          server_host =
              request[boost::beast::http::field::host].substr(0, suffix_pos).to_string();
          logger.logRecvReq(request["requestID"].to_string(),
                            request.method_string().to_string() + " " +
                                request.target().to_string() + " " +
                                std::to_string(request.version()),
                            client_ip);

          if (request.method() == boost::beast::http::verb::connect) {
            tunnel = std::unique_ptr<ConnectTunnel>(new ConnectTunnel(
                client, server, request["requestID"].to_string(), logger));
            tunnel->start(self, server_host);
          }
          else if (request.method() == boost::beast::http::verb::post) {
            connectOriginServer();
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
          if (ec != boost::beast::http::error::end_of_stream) {
            send400ToClient();
          }
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
          logger.logRequesting(std::string(request["requestID"]),
                               request.method_string().to_string() + " " +
                                   request.target().to_string() + " " +
                                   std::to_string(request.version()),
                               server_host);
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
          logger.logRecvRes(request["requestID"].to_string(),
                            std::to_string(response.version()) + " " +
                                std::to_string(response.result_int()) + " " +
                                response.reason().to_string(),
                            server_host);
          updateCache();
        }
        else {
          std::cerr << "recvResFrOriginServer() ec: " << ec.message() << "\n["
                    << response.base() << "]\n";
          send502ToClient();
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
        if (!ec) {
          logger.logResponding(request["requestID"].to_string(),
                               std::to_string(response.version()) + " " +
                                   std::to_string(response.result_int()) + " " +
                                   response.reason().to_string());
          recvReqFrClient();
        }
        else {
          std::cerr << "sendResToClient() ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::connectOriginServer() {
  boost::system::error_code ec;
  // Make the connection on the IP address we get from a lookup
  auto ipResolver = boost::asio::ip::tcp::resolver(server.get_executor());
  auto endpoints = ipResolver.resolve(server_host, "http", ec);
  if (ec) {
    std::cerr << "connectOriginServer(" << server_host << ":80) ec: " << ec.message()
              << "\n";
  }
  auto self = shared_from_this();
  boost::asio::async_connect(
      server,
      endpoints,
      [this, self](boost::system::error_code ec, boost::asio::ip::tcp::endpoint ep) {
        // std::cout << "(thr_id=" << std::this_thread::get_id() << ") ";
        if (!ec) {
          sendReqToOriginServer();
        }
        else {
          std::cerr << "connectOriginServer(" << server_host
                    << ":80) ec: " << ec.message() << "\n";
        }
      });
}

void ProxySession::lookupCache() {
  // if (!cache.checkResExist(req.target)) { connectOriginServer(host); }
  // else {
  //  if (cache.checkValidate(req.target)) { response = cache.getResponse(req.target); }
  //  else { request.set( cache.getNewDate(req.target) ); connectOriginServer(host); )
  // }
  connectOriginServer();
}

void ProxySession::updateCache() {
  // if (response.status == 304) { response = cache.updateResponse(req.target, response))}
  // else if (response.status == 200) { cache.updateResponse(req.target, response)}
  sendResToClient();
}

void ProxySession::send400ToClient() {
  response = {};
  response.version(11);
  response.result(boost::beast::http::status::bad_request);
  sendResToClient();
}

void ProxySession::send502ToClient() {
  response = {};
  response.version(11);
  response.result(boost::beast::http::status::bad_gateway);
  sendResToClient();
}
/*------------------------------------  EOF  ---------------------------------------*/