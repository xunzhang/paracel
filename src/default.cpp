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
#include "paracel_types.hpp"
#include "proxy.hpp"

int paracel_incr_i(int a, int b) {
  return a + b;
}

float paracel_incr_f(float a, float b) {
  return a + b;
}

double paracel_incr_d(double a, double b) {
  return a + b;
}

paracel::list_type<int> paracel_incr_li(paracel::list_type<int> a, paracel::list_type<int> b) {
  paracel::list_type<int> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
}

paracel::list_type<float> paracel_incr_lf(paracel::list_type<float> a, paracel::list_type<float> b) {
  paracel::list_type<float> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
}

paracel::list_type<double> paracel_incr_ld(paracel::list_type<double> a, paracel::list_type<double> b) {
  paracel::list_type<double> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
}

update_result default_incr_i = paracel::update_proxy(paracel_incr_i);
update_result default_incr_f = paracel::update_proxy(paracel_incr_f);
update_result default_incr_d = paracel::update_proxy(paracel_incr_d);
update_result default_incr_li = paracel::update_proxy(paracel_incr_li);
update_result default_incr_lf = paracel::update_proxy(paracel_incr_lf);
update_result default_incr_ld = paracel::update_proxy(paracel_incr_ld);
