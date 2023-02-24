/*------------------------------  ConnectTunnel.hpp  ---------------------------------*/
#ifndef CONNECT_TUNNEL_HPP
#define CONNECT_TUNNEL_HPP

#include <boost/asio.hpp>

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>

class ConnectTunnel {
  std::weak_ptr<void> session_ptr;
  std::array<char, 65536> bytes_buf;
};

#endif
/*------------------------------------  EOF  ---------------------------------------*/