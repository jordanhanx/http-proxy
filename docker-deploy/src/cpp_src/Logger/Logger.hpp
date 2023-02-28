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
  std::ofstream log_file;
  std::mutex mtx;

  static std::string getUTCNow();
  static std::string getHTTPVersion(unsigned int version_int);

 public:
  Logger(const std::string & path);
  ~Logger();

  void log(const std::string & s);

  void logRecvReq(const std::string & request_id,
                  const std::string & request_method,
                  const std::string & request_target,
                  unsigned int request_version,
                  const std::string & from);

  void logRequesting(const std::string & request_id,
                     const std::string & request_method,
                     const std::string & request_target,
                     unsigned int request_version,
                     const std::string & server);

  void logRecvRes(const std::string & request_id,
                  unsigned int response_version,
                  unsigned int response_result_int,
                  const std::string & response_reason,
                  const std::string & server);

  void logResponding(const std::string & request_id,
                     unsigned int response_version,
                     unsigned int response_result_int,
                     const std::string & response_reason);
};
#endif
/*------------------------------------  EOF  ---------------------------------------*/