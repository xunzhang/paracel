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
#include "paracel_types.hpp"

extern "C" {
  extern paracel::update_result hostname_updater;
}

std::string local_update(const std::string & a, const std::string & b) {
  return a + "," + b;
}

paracel::update_result hostname_updater = paracel::update_proxy(local_update);
