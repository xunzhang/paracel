/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
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
}

int local_update(int a, int b) {
  return a + b;
}

paracel::update_result wc_updater = paracel::update_proxy(local_update);
