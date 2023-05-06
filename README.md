# Asynchronous multi-thread HTTP Proxy
This GitHub repository contains a course project that implements an HTTP proxy server using Boost.Asio, a powerful asynchronous I/O library that utilizes the proactor pattern. The purpose of the proxy server is to forward client requests to the origin server on behalf of the client. The repository contains the source code for the proxy server, along with any necessary configuration files and build scripts. The project is structured in a way that separates the server logic from the networking logic, making it modular and easier to maintain. The use of Boost.Asio allows for high-performance, non-blocking I/O that can handle multiple client connections at once. 
### Compile
```
cd ./docker-deploy/src/
make
```
### Run
```
./docker-deploy/src/cpp_build/http-caching-proxy <port> <threads>
```

### Hierarchy
```
docker-deploy
│
├── docker-compose.yml
│
├── src
│   ├── Dockerfile
│   ├── launch.sh
│   ├── Makefile
│   │
│   ├── cpp_src
│   │    ├── Cache
│   │    ├── ConnectTunnel
│   │    ├── Logger
│   │    ├── ProxyServer
│   │    ├── ProxySession
│   |    └── http-caching-proxy.cpp
│   |
|   └── cpp_build
│        ├── objs
|        └── http-caching-proxy

```

