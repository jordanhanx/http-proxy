/*------------------------------  ConnectTunnel.hpp  ---------------------------------*/
#ifndef CONNECT_TUNNEL_HPP
#define CONNECT_TUNNEL_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

class ConnectTunnel {
 private:
  std::shared_ptr<void> keepSessionAlive;
  boost::asio::ip::tcp::socket & client;
  boost::asio::ip::tcp::socket & server;
  std::array<char, 65536> upstream_buf;
  std::array<char, 65536> downstream_buf;

  void connectToServer(const std::string & host);
  void send200okToClient();
  // upstream
  void recvBytesFrClient();
  void sendBytesToOriginServer(std::size_t bytes_transferred);
  // downstream
  void recvBytesFrOriginServer();
  void sendBytesToClient(std::size_t bytes_transferred);

 public:
  ConnectTunnel(boost::asio::ip::tcp::socket & client,
                boost::asio::ip::tcp::socket & server);
  ~ConnectTunnel();

  void start(std::shared_ptr<void> session, const std::string & host);
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/