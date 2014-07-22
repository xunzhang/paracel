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
#include "utils.hpp"
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::string;

extern "C" {
  extern paracel::filter_with_key_result W_filter;
}

bool local_filter_W(const std::string & key) {
  if(paracel::startswith(key, "W_")) {
    return true;
  }
  return false;
}

paracel::filter_with_key_result W_filter = paracel::filter_with_key_proxy(local_filter_W);
