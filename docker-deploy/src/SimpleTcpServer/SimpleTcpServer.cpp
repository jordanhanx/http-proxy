/*------------------------------  SimpleTcpServer.cpp  ---------------------------------*/
#include "SimpleTcpServer.hpp"

SimpleTcpServer::SimpleTcpServer(const char * port) : socket(port), out(std::cout) {
}

void SimpleTcpServer::runServer() {
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