/*------------------------------  ProxySession.hpp  ---------------------------------*/
#ifndef PROXY_SESSION_HPP
#define PROXY_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>

#include "../ConnectTunnel/ConnectTunnel.hpp"

class ProxySession : public std::enable_shared_from_this<ProxySession> {
 private:
  boost::asio::ip::tcp::socket client;
  boost::asio::ip::tcp::socket server;
  std::string client_address;
  std::string server_address;

  ConnectTunnel tunnel;

  boost::asio::streambuf buf;  // (Must persist between reads)
  boost::beast::http::request<boost::beast::http::empty_body> request;
  boost::beast::http::response<boost::beast::http::string_body> response;

  void recvReqFrClient();
  void sendReqToOriginServer();
  void recvResFrOriginServer();
  void sendResToClient();

  void connectOriginServer(const std::string & host);

  void lookupCache();
  void updateCache();

 public:
  explicit ProxySession(boost::asio::ip::tcp::socket socket);
  ~ProxySession();
  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/