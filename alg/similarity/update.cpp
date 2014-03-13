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
#include <utility>
#include <string>
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;
using std::pair;
using std::string;

extern "C" {
  extern paracel::update_result sim_updater;
}

vector<pair<string, double> > 
local_update(const vector<pair<string, double> > & a,
			const vector<pair<string, double> > & b) {
  vector<pair<string, double> > r(a);
  r.insert(r.end(), b.begin(), b.end());
  return r;
}

paracel::update_result sim_updater = paracel::update_proxy(local_update);
