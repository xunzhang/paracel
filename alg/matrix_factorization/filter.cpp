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

#include <string>
#include "proxy.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

using namespace std;

extern "C" {
  extern paracel::filter_with_key_result mf_ubias_filter;
  extern paracel::filter_with_key_result mf_ibias_filter;
  extern paracel::filter_with_key_result mf_W_filter;
  extern paracel::filter_with_key_result mf_H_filter;
}

bool filter_ubias(const std::string & key) {
  string s = "usr_bias[";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

bool filter_ibias(const std::string & key) {
  string s = "item_bias[";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

bool filter_W(const std::string & key) {
  string s = "W[";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

bool filter_H(const std::string & key) {
  string s = "H[";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

paracel::filter_with_key_result mf_ubias_filter = paracel::filter_with_key_proxy(filter_ubias);
paracel::filter_with_key_result mf_ibias_filter = paracel::filter_with_key_proxy(filter_ibias);
paracel::filter_with_key_result mf_W_filter = paracel::filter_with_key_proxy(filter_W);
paracel::filter_with_key_result mf_H_filter = paracel::filter_with_key_proxy(filter_H);
