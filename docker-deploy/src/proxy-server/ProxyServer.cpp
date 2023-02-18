/*------------------------------  ProxyServer.hpp  ---------------------------------*/
#include "ProxyServer.hpp"

ProxyServer::ProxyServer() : out(std::cout) {
}

ProxyServer::ProxyServer(const char * port) : socket(port), out(std::cout) {
}

ProxyServer::ProxyServer(const char * port, std::ostream & out) : socket(port), out(out) {
}

ProxyServer::~ProxyServer() {
}

void ProxyServer::runServer() {
  try {
    socket.listenOnPort();
    out << "Server: waiting for connections on " + socket.getSocketIP() + "\n";
    socket.acceptConn();
    out << "server: got connection from " + socket.getListenIP() + "\n";
    while (true) {
      out << socket.recvLine();
    }
  }
  catch (const SocketError & e) {
    out << e.what() << "\n";
  }
}
/*------------------------------------  EOF  ---------------------------------------*/