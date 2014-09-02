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

#include <vector>
#include "proxy.hpp"
#include "paracel_types.hpp"

extern "C" {
  extern paracel::update_result perf_updater;
}

std::vector<double> local_update(const std::vector<double> & a, 
                                 const std::vector<double> & b) {
  std::vector<double> r(a);
  return r;
}

paracel::update_result perf_updater = paracel::update_proxy(local_update);
