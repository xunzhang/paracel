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
#include <string>
#include "utils.hpp"
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;
using std::pair;
using std::string;

extern "C" {
  extern paracel::update_result init_updater;
  extern paracel::filter_with_key_result pr_filter;
}

vector<pair<string, double> >
local_update(const vector<pair<string, double> > & a,
             const vector<pair<string, double> > & b) {
  vector<pair<string, double> > r(a);
  r.insert(r.end(), b.begin(), b.end());
  return r;
}

bool local_filter(const std::string & key) {
  string s = "_pr";
  if(paracel::endswith(key, s)) {
    return true;
  }
  return false;
}

paracel::update_result init_updater = paracel::update_proxy(local_update);
paracel::filter_with_key_result pr_filter = paracel::filter_with_key_proxy(local_filter);
