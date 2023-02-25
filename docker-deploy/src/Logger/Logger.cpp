/*------------------------------  Logger.cpp  ---------------------------------*/
#include "Logger.hpp"

Logger::Logger(const std::string & path) : path(path) {
  if (!myFile.is_open()) {
    myFile.open(path, std::fstream::app);
  }
}
Logger::~Logger() {
  if (myFile.is_open()) {
    myFile.flush();
    myFile.close();
  }
}

std::string Logger::getUTCNow() {  // return a string ending with a '\n'
  auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return std::asctime(std::gmtime(&time));
}

void Logger::log(const std::string & s) {
  std::lock_guard<std::mutex> lck(mtx);
  myFile << s << std::endl;
}

void Logger::logRecvReq(const std::string & requestID,
                        const std::string & requestHeader,
                        const std::string & from) {
  std::lock_guard<std::mutex> lck(mtx);
  myFile << requestID << ": \"" << requestHeader << "\" from " << from << " @ "
         << getUTCNow() << std::flush;
}
void Logger::logRequesting(const std::string & requestID,
                           const std::string & requestHeader,
                           const std::string & from) {
  std::lock_guard<std::mutex> lck(mtx);
  myFile << requestID << ": "
         << "Requesting \"" << requestHeader << "\" from " << from << std::endl;
}

void Logger::logRecvRes(const std::string & requestID,
                        const std::string & responseHeader,
                        const std::string & from) {
  std::lock_guard<std::mutex> lck(mtx);
  myFile << requestID << ": "
         << "Received \"" << responseHeader << "\" from " << from << std::endl;
}

void Logger::logResponding(const std::string & requestID,
                           const std::string & responseHeader) {
  std::lock_guard<std::mutex> lck(mtx);
  myFile << requestID << ": "
         << "Responding \"" << responseHeader << "\"" << std::endl;
}

/*---------------------------------  EOF  -------------------------------------*/