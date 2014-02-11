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
#include <string>
#include <functional>
#include "proxy.hpp"

using std::string;
using update_result = std::function<string(string, string)>;

extern "C" {
  extern update_result sgd_theta_update;
}

vector<double> local_update(const vector<double> & a, const vector<double> & b) {
  vector<double> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
}

update_result sgd_theta_update = paracel::update_proxy(local_update);
