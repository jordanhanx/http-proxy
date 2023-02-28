/*------------------------------  ProxySession.hpp  ---------------------------------*/
#ifndef PROXY_SESSION_HPP
#define PROXY_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "../ConnectTunnel/ConnectTunnel.hpp"
#include "../Logger/Logger.hpp"
#include "../Cache/Cache.hpp"

class ProxySession : public std::enable_shared_from_this<ProxySession> {
 private:
  static std::atomic<size_t> next_request_id;

  boost::asio::ip::tcp::socket client;
  boost::asio::ip::tcp::socket server;

  Logger & logger;
  Cache & cache;

  std::string client_ip;
  std::string server_host;

  boost::asio::streambuf buf;
  boost::beast::http::request<boost::beast::http::string_body> request;
  boost::beast::http::response<boost::beast::http::string_body> response;

  std::unique_ptr<ConnectTunnel> tunnel;  // lazy construct

  void recvReqFrClient();
  void sendReqToOriginServer();
  void recvResFrOriginServer();
  void sendResToClient();

  void connectOriginServer();

  void lookupCache();
  void updateCache();

  void send400ToClient();
  void send502ToClient();

 public:
  ProxySession(boost::asio::ip::tcp::socket socket, Logger & logger, Cache & cache);
  ~ProxySession();
  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/