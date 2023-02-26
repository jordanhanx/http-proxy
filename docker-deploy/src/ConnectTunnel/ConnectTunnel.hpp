/*------------------------------  ConnectTunnel.hpp  ---------------------------------*/
#ifndef CONNECT_TUNNEL_HPP
#define CONNECT_TUNNEL_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "../Logger/Logger.hpp"

class ConnectTunnel {
 private:
  std::weak_ptr<void> session_life_tracker;
  boost::asio::ip::tcp::socket & client;
  boost::asio::ip::tcp::socket & server;
  std::string request_id;
  std::string server_host;
  Logger & logger;

  std::array<char, 65536> upstream_buf;
  std::array<char, 65536> downstream_buf;

  void connectToServer();
  void send200okToClient();
  // upstream
  void recvBytesFrClient();
  void sendBytesToOriginServer(std::size_t bytes_transferred);
  // downstream
  void recvBytesFrOriginServer();
  void sendBytesToClient(std::size_t bytes_transferred);

 public:
  ConnectTunnel(std::shared_ptr<void> session,
                boost::asio::ip::tcp::socket & client,
                boost::asio::ip::tcp::socket & server,
                const std::string & request_id,
                const std::string & server_host,
                Logger & logger);
  ~ConnectTunnel();

  void start();
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/