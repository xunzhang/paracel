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
  extern paracel::update_result mf_fac_updater;
  extern paracel::update_result mf_bias_updater;
}

vector<double> local_update_fac(vector<double> a, vector<double> b) {
  vector<double> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
  return r;
}

double local_update_bias(double a, double b) {
  return a + b;
}

paracel::update_result mf_fac_updater = paracel::update_proxy(local_update_fac);
paracel::update_result mf_bias_updater = paracel::update_proxy(local_update_bias);
