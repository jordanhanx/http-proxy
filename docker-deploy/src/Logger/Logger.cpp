/*------------------------------  Logger.cpp  ---------------------------------*/
#include "Logger.hpp"

Logger::Logger(const std::string & path) : path(path) {
  if (!log_file.is_open()) {
    log_file.open(path, std::fstream::app);
  }
}
Logger::~Logger() {
  if (log_file.is_open()) {
    log_file.flush();
    log_file.close();
  }
}

std::string Logger::getUTCNow() {  // return a string ending with a '\n'
  auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return std::asctime(std::gmtime(&time));
}

std::string Logger::getHTTPVersion(unsigned int version_int) {
  return "HTTP/" + std::to_string(version_int / 10) + "." +
         std::to_string(version_int % 10);
}

void Logger::log(const std::string & s) {
  std::lock_guard<std::mutex> lck(mtx);
  log_file << s << std::endl;
}

void Logger::logRecvReq(const std::string & request_id,
                        const std::string & request_method,
                        const std::string & request_target,
                        unsigned int request_version,
                        const std::string & from) {
  std::lock_guard<std::mutex> lck(mtx);
  log_file << request_id << ": \"" << request_method << " " << request_target << " "
           << getHTTPVersion(request_version) << "\" from " << from << " @ "
           << getUTCNow() << std::flush;
}
void Logger::logRequesting(const std::string & request_id,
                           const std::string & request_method,
                           const std::string & request_target,
                           unsigned int request_version,
                           const std::string & server) {
  std::lock_guard<std::mutex> lck(mtx);
  log_file << request_id << ": "
           << "Requesting \"" << request_method << " " << request_target << " "
           << getHTTPVersion(request_version) << "\" from " << server << std::endl;
}

void Logger::logRecvRes(const std::string & request_id,
                        unsigned int response_version,
                        unsigned int response_result_int,
                        const std::string & response_reason,
                        const std::string & server) {
  std::lock_guard<std::mutex> lck(mtx);

  log_file << request_id << ": "
           << "Received \"" << getHTTPVersion(response_version) << " "
           << response_result_int << " " << response_reason << "\" from " << server
           << std::endl;
}

void Logger::logResponding(const std::string & request_id,
                           unsigned int response_version,
                           unsigned int response_result_int,
                           const std::string & response_reason) {
  std::lock_guard<std::mutex> lck(mtx);
  log_file << request_id << ": "
           << "Responding \"" << getHTTPVersion(response_version) << " "
           << response_result_int << " " << response_reason << "\"" << std::endl;
}

/*---------------------------------  EOF  -------------------------------------*/