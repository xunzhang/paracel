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
#ifndef FILE_56636034_fc34_4f9e_4343_4111ca3b127d_HPP
#define FILE_56636034_fc34_4f9e_4343_4111ca3b127d_HPP

#include <map>
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

// ['a', 'c', 'b', 'a', 'a', 'b'] -> [3, 2, 1]
paracel::list_type<int> sort_and_cnt(const paracel::list_type<paracel::str_type> & in) {
  paracel::list_type<int> r;
  std::map<paracel::str_type, int> m;
  for(auto & str : in) {
    if(m.find(str) == m.end()) {
      m[str] = 1;
    } else {
      m[str] += 1;
    }
  }
  for(auto & mp : m) {
    r.push_back(mp.second);
  }
  return r;
}

} // namespace paracel
#endif
