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
#ifndef FILE_756990b7_918d_30f3_cf76_7c5e5a1bc9e1_HPP
#define FILE_756990b7_918d_30f3_cf76_7c5e5a1bc9e1_HPP

#include "paracel_types.hpp"

namespace paracel {

typedef paracel::list_type< paracel::str_type > slst_type;

slst_type str_split(const paracel::str_type & str, const char sep) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s != "") result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

slst_type str_split(paracel::str_type && str, const char sep) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s != "") result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}


slst_type str_split(const paracel::str_type & str, const paracel::str_type & seps) {}

slst_type str_split(paracel::str_type && str, const paracel::str_type & seps) {}

}

#endif
