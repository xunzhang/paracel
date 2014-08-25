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

using std::vector;

extern "C" {
  extern paracel::update_result wc_updater;
  extern paracel::update_result wc_update_test;
}

int local_update(int a, int b) {
  return a + b;
}

std::vector<std::string> update_test(const std::vector<std::string> & a,
                                     const std::vector<std::string> & b) {
  std::vector<std::string> r(a);
  r.insert(r.end(), b.begin(), b.end());
  return r;
}

paracel::update_result wc_updater = paracel::update_proxy(local_update);
paracel::update_result wc_update_test = paracel::update_proxy(update_test);
