#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include "Cache.hpp"

bool Cache::checkResExist(const std::string& url){
  std::cout << "current size: " << cache_size << std::endl;
  bool is_exist = (get_cache_idx(url) == -1) ? false : true;
  return is_exist;
}

bool Cache::checkValidate(const std::string& url){
  std::cout << "current size: " << cache_size << std::endl;
  http::response<http::string_body> res = find_res(url);
  std::string expires = std::string(res.base()[http::field::expires]);
  const std::string cache_control = std::string(res.base()[http::field::cache_control]);
  return need_revalidate(url, expires, cache_control);
}

http::response<http::string_body> Cache::getResponse(const std::string& url){
  std::cout << "getResponse " << cache_size << std::endl;
  return find_res(url);
}

std::string Cache::getCahchedDate(const std::string& url){
  std::cout << "current size: " << cache_size << std::endl;
  http::response<http::string_body> res = find_res(url);
  std::string cached_date = std::string(res.base()[http::field::date]);
  return cached_date;
}

//response
http::response<http::string_body> Cache::updateResponse(http::response<http::string_body> update, const std::string& url, bool updateHeader, Logger& log){
  std::cout << "updateResponse " << cache_size << std::endl;
  std::cout << "current size: " << cache_size << std::endl;
   if(updateHeader){
    std::cout << "update header: " <<  find_res(url) << std::endl;
    // http::response<http::string_body> original = ;
    find_res(url).base() = update.base();
    std::cout << "original base : " <<   find_res(url).base() << std::endl;
    return find_res(url);
   }else{
    if(get_cache_idx(url) != -1){
      std::cout << "find url in cache" << std::endl;
      find_res(url)  = update;
      return update;
    }
      handle_200(update, url, log);
      std::cout << "finish handle_200"<< std::endl;
      printCache();
      printCacheLookUp();
    return update;
   }  
}

void Cache::printCache(){
  // run for loop from 0 to vecSize
   std::cout << "print cache " << std::endl;
  for(int i=0; i<cache_size; i++)
  {
    std::cout << "idx: " << i << std::endl;
    std::cout << "request_id: " << cache[i]["request_id"] << std::endl;
  }
}

void Cache::printCacheLookUp(){
  // run for loop from 0 to vecSize
  std::cout << "print cache_lookup " << std::endl;
  for (auto i = cache_lookup.begin(); i != cache_lookup.end(); i++)
  std::cout << i->first << "       " << i->second << std::endl;
}

//find target response in cache using url
void Cache::handle_200(http::response<http::string_body> new_res, const std::string& url, Logger & logger){
  std::lock_guard<std::mutex> lck(mtx);
  //check if new_res existed
  std::cout << "check cache not exist" << std::endl;
  //not exist
  if(can_be_cached(new_res)){
    if(cache_size == max_size){
      std::cout << "cache full" << std::endl;
      curr = curr % max_size;
      std::cout << "check 1" << std::endl;
      auto it = std::find_if(std::begin(cache_lookup), std::end(cache_lookup),
                         [&](const std::pair<std::string,int>& p)->bool { return p.second == curr; });
      std::cout << "check 2" << std::endl;
      std::cout << it->second["request_id"] << std::endl;
      cache_lookup.erase(it); 
      std::cout << "check 3" << std::endl;
      cache[curr] = new_res;
      std::cout << "check 4" << std::endl;

      std::cout << "cache look up remove element and cache update element" << std::endl;
    }else{
      std::cout << "cache not full" << std::endl;
      cache.push_back(new_res);
      std::cout << "add cache" << std::endl;
      cache_size++;
    }
    // int idx = (cache_size <= max_size) ? cache_size-1:curr;
    cache_lookup.insert({url, curr});
    curr++;
    std::cout << "cache look up add 1" << std::endl;
    //Log revalidation
    std::string expires = std::string(new_res.base()[http::field::expires]);
    const std::string cache_control = std::string(new_res.base()[http::field::cache_control]);
    if(need_revalidate(url, expires, cache_control)){
      //LOG message: cached, but requires re-validation
      logger.log(new_res["request_id"].to_string() + ": cached, but requires re-validation");
      std::cout<< "cached, but requires re-validation" << std::endl;
    }else{
      if(!expires.empty()){
        //LOG: cached, expires at EXPIRES
        logger.log(new_res["request_id"].to_string() + ": cached, but expires at " + expires);
        std::cout<< "cached, expires at " << expires << std::endl;
      }
      logger.log(new_res["request_id"].to_string()+ ": cached" + expires);
      std::cout<< "cached" << std::endl;
    }
  }else{
    //can not cache
    //LOG: not cacheable because REASON
    logger.log(new_res["request_id"].to_string() + ": not cacheable");
    std::cout<< "not cacheable" << std::endl;
    return;
  }
}


int Cache::get_cache_idx(const std::string& url){
  std::cout << "find url: " << url << std::endl;
  if(cache_lookup.find(url) == cache_lookup.end()){
    std::cout << "not found" << std::endl;
    return -1;
  }else{
    std::cout << "find url with index " << cache_lookup.at(url) << std::endl;
    return cache_lookup.at(url);
  }
  
}

http::response<http::string_body> Cache::find_res(const std::string& url){
  std::lock_guard<std::mutex> lck(mtx);
  std::cout << "current size: " << cache_size << std::endl;
  return cache[get_cache_idx(url)];
}

//if response has private or no store in cache-control field, or max-age, expires equals -1, it cannot be cached
bool Cache::can_be_cached(http::response<http::string_body> res){
  std::cout << "start can be cache" << std::endl;
  bool can_cache = true;
  const auto cache_control = std::string(res.base()[http::field::cache_control]);
  const auto is_private = std::string("private");
  const auto no_store = std::string("no-store");
  std::string expires = std::string(res.base()[http::field::expires]);
  int max_age_idx = (cache_control.find(std::string("max-age"))!= std::string::npos) ? cache_control.find(std::string("max-age")): -1;
    // std::cout << max_age_idx << std::endl;
  const auto max_age_val = (max_age_idx != -1) ? cache_control.substr(max_age_idx+8, 2): " ";
  if(boost::algorithm::contains(cache_control, is_private) 
     || boost::algorithm::contains(cache_control, no_store)
     || beast::iequals(expires, "-1")
     || beast::iequals(max_age_val, "-1")){
      std::cout << "can not cache" << std::endl;
      return !can_cache;
  }
  return can_cache;
}

//check if resposne has field must-revalidate or no-cache, need revalidate
bool Cache::need_revalidate(const std::string& url, const std::string& expires, const std::string& cache_control){
  bool need = true;
  std::string need_revalidate = "must-revalidate";
  std::string no_cache = "no-cache";
  std::time_t curr_time = get_curr_time();
  if(!(expires.empty() && cache_control.empty())){
    std::tm expired_time;
    strptime(expires.substr(0, expires.size()-3).c_str(), "%a,%d %b %Y %H:%M:%S", &expired_time);
    std::time_t e_time = mktime(&expired_time);
    if(boost::algorithm::contains(cache_control, need_revalidate)
      || boost::algorithm::contains(cache_control, no_cache)
      || difftime(e_time, curr_time) < 0){
            std::cout << "need revalidate" << std::endl;
            return need;
    }
  }
  std::cout << "don't need revalidate" << std::endl;
  return !need;
}


std::time_t Cache::get_curr_time(){
  auto curr_time = std::chrono::system_clock::now();
  std::time_t end_time = std::chrono::system_clock::to_time_t(curr_time);
  std::tm convert = *std::localtime(&end_time);
  convert.tm_hour += 5;
  std::time_t gmt_time = mktime(&convert);
  return gmt_time;
}

Cache::~Cache(){}