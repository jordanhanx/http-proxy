#ifndef CACHE_HPP
#define CACHE_HPP

#include <boost/algorithm/string.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

#include "../Logger/Logger.hpp"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>

class Cache {
 public:
  Cache() : cache_size(0), max_size(20), curr(0){};
  bool checkResExist(const std::string & url);
  bool checkValidate(const std::string & url, Logger & log);
  http::response<http::string_body> getResponse(const std::string & url);
  std::string getCahchedDate(const std::string & url);
  void printCache();
  void printCacheLookUp();
  http::response<http::string_body> updateResponse(
      http::response<http::string_body> update,
      const std::string & url,
      bool updateHeader,
      Logger & log);
  std::mutex mtx;
  ~Cache();

 private:
  std::unordered_map<std::string, int> cache_lookup;
  std::vector<http::response<http::string_body> > cache;
  int cache_size;
  int max_size;
  int curr;

  http::response<http::string_body> find_res(const std::string & url);
  void handle_200(http::response<http::string_body> new_res,
                  const std::string & url,
                  Logger & log);
  int get_cache_idx(const std::string & url);
  bool can_be_cached(http::response<http::string_body> res, Logger & logger);
  bool need_revalidate(const std::string & url,
                       const std::string & expires,
                       const std::string & cache_control);
  bool check_expired(const std::string & url, const std::string & expires);
  std::time_t get_curr_time();
};

#endif
