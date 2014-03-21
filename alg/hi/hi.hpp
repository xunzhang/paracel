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

#ifndef FILE_49843a5e_b2f8_f70c_58c7_1b13de85a6eb_HPP 
#define FILE_49843a5e_b2f8_f70c_58c7_1b13de85a6eb_HPP 

#include <iostream>
#include "ps.hpp"

namespace paracel {

class hi : public paracel::paralg {

public:
  hi(paracel::Comm comm, std::string hosts_dct_str,
  	std::string _input, std::string _output, std::string para) :
	paracel::paralg(hosts_dct_str, comm, _output, 1, 0, true), 
	input(_input),
	hi_para(para) {}

  virtual ~hi() {}

  virtual void solve() {
    std::cout << "a" << input << std::endl;
    auto lambda = [&] (paracel::list_type<paracel::str_type> & somelines) {
      for(auto & line : somelines) {
	    if(get_worker_id() == 0) {
	      std::cout << line << std::endl;
		}
	  }
	  if(get_worker_id() == 0) {
	    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
	  }
	};
    paracel_sequential_loadall(input, lambda);
  }

private:
  std::string input;
  std::string hi_para;
};

}

#endif
