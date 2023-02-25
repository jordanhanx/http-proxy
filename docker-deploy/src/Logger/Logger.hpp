/*------------------------------  Logger.hpp  ---------------------------------*/
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>

class Logger {
 private:
  const std::string path;
  std::ofstream myFile;
  std::mutex mtx;

  static std::string getUTCNow();

 public:
  Logger(const std::string & path);
  ~Logger();

  void log(const std::string & s);

  void logRecvReq(const std::string & requestID,
                  const std::string & requestHeader,
                  const std::string & from);

  void logRequesting(const std::string & requestID,
                     const std::string & requestHeader,
                     const std::string & from);

  void logRecvRes(const std::string & requestID,
                  const std::string & responseHeader,
                  const std::string & from);

  void logResponding(const std::string & requestID, const std::string & responseHeader);
};
#endif
/*------------------------------------  EOF  ---------------------------------------*/