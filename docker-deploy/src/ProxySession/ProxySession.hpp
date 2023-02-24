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

class ProxySession : public std::enable_shared_from_this<ProxySession> {
 private:
  boost::asio::ip::tcp::socket client_socket;
  boost::asio::ip::tcp::socket server_socket;

  std::string client_address;
  std::string server_address;

  boost::asio::streambuf buf;  // (Must persist between reads)
  std::array<uint8_t, 65536> bytes_buf;

  boost::beast::http::request<boost::beast::http::empty_body> request;
  boost::beast::http::response<boost::beast::http::string_body> response;

  void recvReqFrClient();
  void sendReqToOriginServer();
  void recvResFrOriginServer();
  void sendResToClient();

  void connectOriginServer(const std::string & host, const std::string & port);
  void send200okToClient();

  void recvBytesFrClient();
  void sendBytesToOriginServer(std::size_t bytes_transferred);
  void recvBytesFrOriginServer();
  void sendBytesToClient(std::size_t bytes_transferred);

  void lookupCache(const std::string & host, const std::string & port);
  void updateCache();

 public:
  ProxySession(boost::asio::ip::tcp::socket socket);
  ~ProxySession();
  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/