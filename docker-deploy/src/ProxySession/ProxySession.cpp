/*------------------------------  ProxySession.cpp  ---------------------------------*/
#include "ProxySession.hpp"

std::atomic<size_t> ProxySession::next_request_id(1);

ProxySession::ProxySession(boost::asio::ip::tcp::socket socket, Logger & logger, Cache & cache) :
    client(std::move(socket)), server(socket.get_executor()), logger(logger), cache(cache){
  client_ip = client.remote_endpoint().address().to_string();
}

ProxySession::~ProxySession() {
}

void ProxySession::start() {
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
        request.set("request_id", next_request_id);
        next_request_id++;
        if (!ec) {
          auto suffix_pos = request[boost::beast::http::field::host].rfind(":443");
          server_host =
              request[boost::beast::http::field::host].substr(0, suffix_pos).to_string();
          logger.logRecvReq(request["request_id"].to_string(),
                            request.method_string().to_string(),
                            request.target().to_string(),
                            request.version(),
                            client_ip);

          if (request.method() == boost::beast::http::verb::connect) {
            tunnel = std::unique_ptr<ConnectTunnel>(
                new ConnectTunnel(self,
                                  client,
                                  server,
                                  request["request_id"].to_string(),
                                  server_host,
                                  logger));
            tunnel->start();
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
        if (!ec) {
          logger.logRequesting(std::string(request["request_id"]),
                               request.method_string().to_string(),
                               request.target().to_string(),
                               request.version(),
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
        if (!ec) {
          response.set("request_id", request["request_id"]);
          logger.logRecvRes(response["request_id"].to_string(),
                            response.version(),
                            response.result_int(),
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
        if (!ec) {
          logger.logResponding(response["request_id"].to_string(),
                               response.version(),
                               response.result_int(),
                               response.reason().to_string());
          // recvReqFrClient();
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
        if (!ec) {
          sendReqToOriginServer();
        }
        else {
          send400ToClient();
          std::cerr << "connectOriginServer(" << server_host
                    << ":80) ec: " << ec.message() << "\n";
          
        }
      });
}

void ProxySession::lookupCache() {
  if(!cache.checkResExist(request.target().to_string())){
    logger.log(request["request_id"].to_string() + ": not in cache");
    connectOriginServer();
  }else{
    if(cache.checkValidate(request.target().to_string(), logger)){
      response = cache.getResponse(request.target().to_string());
      sendResToClient();
    }else{
      request.set(http::field::if_modified_since, cache.getCahchedDate(request.target().to_string()));
      connectOriginServer();
    }
  }
}

void ProxySession::updateCache() {
  if(response.result_int() == 304){
    response = cache.updateResponse(response, request.target().to_string(), true, logger);
  }else{
    if(response.result_int() == 200){
      cache.updateResponse(response, request.target().to_string(), false, logger);
    }
  }
  sendResToClient();
}

void ProxySession::send400ToClient() {
  response = {};
  response.version(11);
  response.result(boost::beast::http::status::bad_request);
  response.body() = "HTTP/1.1 400 Bad Request";
  response.prepare_payload();
  sendResToClient();
}

void ProxySession::send502ToClient() {
  response = {};
  response.version(11);
  response.result(boost::beast::http::status::bad_gateway);
  response.body() = "HTTP/1.1 502 Bad Gateway";
  response.prepare_payload();
  sendResToClient();
}
/*------------------------------------  EOF  ---------------------------------------*/