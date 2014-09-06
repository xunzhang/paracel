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
#include "utils.hpp"

extern "C" {
  extern paracel::update_result cos_sim_sparse_updater;
  extern paracel::filter_with_key_result cos_sim_sparse_filter;
}

std::vector<double> local_update(const std::vector<double> & a, 
                                 const std::vector<double> & b) {
  std::vector<double> r(a);
  for(size_t i = 0; i < b.size(); ++i) {
    r[i] += b[i];
  }
  return r;
}

bool filter(const std::string & key) {
  std::string s = "wgt_";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

paracel::update_result cos_sim_sparse_updater = paracel::update_proxy(local_update);
paracel::filter_with_key_result cos_sim_sparse_filter = paracel::filter_with_key_proxy(filter);
