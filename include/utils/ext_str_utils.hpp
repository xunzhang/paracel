/**
 * Copyright (c) 2013, Douban Inc. 
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
#ifndef FILE_569ed89d_8181_d364_099f_bb1c44d3df66_HPP 
#define FILE_569ed89d_8181_d364_099f_bb1c44d3df66_HPP

#include "paracel_types.hpp"

namespace paracel {

typedef paracel::list_type< paracel::str_type > slst_type;

slst_type str_split(const paracel::str_type & str, char sep) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s != "") result.push_back(s);
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

slst_type str_split(const paracel::str_type & str, const paracel::str_type & seps) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find_first_of(seps, st);
    auto s = str.substr(st, en - st);
    if(s != "") result.push_back(s);
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

} // namespace paracel
#endif
