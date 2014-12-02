/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef FILE_9686d5ae_ba8e_3164_1fce_8dfeaa3df77c_HPP
#define FILE_9686d5ae_ba8e_3164_1fce_8dfeaa3df77c_HPP

#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include "ps.hpp"

namespace paracel {

class tiny_url : public paracel::paralg {
 
 public:
  tiny_url(paracel::Comm comm, 
           std::string hosts_dct_str,
           std::string _input1,
           std::string _input2,
           std::string _output) : paracel::paralg(hosts_dct_str, comm, _output), 
                                  input_originalURLs(_input1),
                                  input_shortURLs(_input2),
                                  output(_output) {
    os1.open(output + "insert_result" + std::to_string(get_comm().get_rank()), std::ofstream::app);
    os2.open(output + "retrive_result" + std::to_string(get_comm().get_rank()), std::ofstream::app);
    for(int i = 0; i < 10; ++i) {
      dictory[i] = i + '0';
    }
    for(int i = 10; i < 36; ++i) {
      dictory[i] = i - 10 + 'a';
    }
    for(int i = 36; i < 62; ++i) {
      dictory[i] = i - 36 + 'A';
    }
    bnd = pow(62, 6);
  }
  
  ~tiny_url() { 
    os1.close();
    os2.close();
  }

  std::string compute(size_t skey) {
    std::string shortURL;
    while(skey > 0) {
      int remainder = skey % 62;
      shortURL.push_back(dictory[remainder]);
      skey = skey / 62;
    }
    while(shortURL.size() < 6) {
      shortURL.push_back('0');
    }
    std::reverse(shortURL.begin(), shortURL.end());
    return shortURL;
  }

  size_t compute(const std::string & shortURL) {
    size_t skey = 0;
    for(int i = 0; i < 6; ++i) {
      char c = shortURL[i];
      int t = 0;
      if(c <= '9' && c >= '0') {
        t = c - '0';
      }
      if(c <= 'z' && c >= 'a') {
        t = c - 'a' + 10;
      }
      if(c <= 'Z' && c >= 'A') {
        t = c - 'A' + 36;
      }
      skey += t * pow(62, 6 - i - 1);
    }
    return skey;
  }
 
  std::string insert(const std::string & longURL) {
    std::string shortURL;
    std::hash<std::string> hf;
    size_t skey = hf(longURL) % bnd;
    paracel_write(std::to_string(skey), longURL);
    shortURL = compute(skey);
    return shortURL;
  }

  std::string retrieve(const std::string & shortURL) {
    size_t skey = compute(shortURL);
    std::string longURL = paracel_read<std::string>(std::to_string(skey));
    return longURL;
  }

  bool check_validate(const std::string & shortURL) {
    std::string prefix = "http://paracel.io/";
    if(startswith(shortURL, prefix) && shortURL.size() == 24) {
      return true;
    }
    return false;
  }

  void solve_insert() {
    // batch insert
    auto lines = paracel_load(input_originalURLs);
    for(auto & line : lines) {
      os1 << line << " -> http://paracel.io/" << insert(line) << '\n';
    }
  }

  void solve_retrieve() {
    // batch retrieve
    auto lines = paracel_load(input_shortURLs);
    for(auto & line : lines) {
      if(!check_validate(line)) { 
        os2 << line << " -> not validate." << '\n';
      } else {
        std::string suffix(line.begin() + 18, line.end());
        os2 << line << " -> " << retrieve(suffix) << '\n';
      }
    }
  }

 private:
  std::string input_originalURLs, input_shortURLs;
  std::string output;
  std::ofstream os1, os2;
  char dictory[62];
  size_t bnd;
}; // class tiny_url

} // namespace paracel

#endif
