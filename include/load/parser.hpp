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
#ifndef FILE_607caade_25c4_da56_7a7b_c0cd1aa35b74_HPP
#define FILE_607caade_25c4_da56_7a7b_c0cd1aa35b74_HPP

#include "paracel_types.hpp"
#include "utils/ext_str_utils.hpp"

namespace paracel {

/**
 * example1: 1001|1|2|4
 *   std::bind(parser_a, std::placeholders::_1, '|')
 *
 * example2: 1001 1 2 4
 *   std::bind(parser_a, std::placeholders::_1)
 *
 */
auto parser_a = 
  [](const paracel::str_type & line, char sep = ' ') {
    auto l = paracel::str_split(line, sep);
    paracel::list_type<int> r;
    for(auto & item : l) {
      r.push_back(std::stoi(item));
    }
    return r;
  }; 

/**
 * example1: a b c d 
 *   std::bind(parser_b, std::placeholders::_1) 
 *   a.txt
 *
 * example2: a b
 *           a c
 *           a d
 *   std::bind(parser_b, std::placeholders::_1)
 *   b.txt
 *
 * example3: a b
 *           a c d
 *   std::bind(parser_b, std::placeholders::_1)
 *   c.txt
 *
 */
auto parser_b = 
  [](const paracel::str_type & line, char sep = ' ') {
    return paracel::str_split(line, sep);
  };

/**
 * example1: a b|c|d
 *           b a|d
 *   std::bind(parser_c, std::placeholders::_1, ' ', '|') 
 *   a2.txt
 *
 * example2: a b:0.1|c:0.2|d:0.4
 *   std::bind(parser_c, std::placeholders::_1, '\t', '|')
 *   f.txt
 *
 * example3: a\tb:0.1
 *           a\tc:0.2 d:0.4
 *   std::bind(parser_c, std::placeholders::_1, '\t')
 *   h.txt
 *
 */
auto parser_c = 
  [](const paracel::str_type & line, 
     char sep1 = ' ', 
     char sep1 = ' ') {
    auto tmp = paracel::str_split(line, sep1);
    auto r = paracel::str_split(tmp[1], sep2);
    r.insert(r.begin(), tmp[0]);
    return r;
  };

/*
auto parser_douban_interest = 
  [](const paracel::str_type & line, char sep = '\t') {
    paracel::list_type
    auto l = paracel::str_split(line, sep);
    if(l[2] == "P") {
      if(l[3] == "NULL" || l[3] == "") {
        // 3.7 is avg interest
	stf = ;
      } else {
        stf = ;
      }
    }
    return stf;
  };

auto parser_netflix = 
  []() {};
*/

} // namespace paracel

#endif
