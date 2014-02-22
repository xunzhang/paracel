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

#ifndef FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP
#define FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP

#include <vector>
#include <string>
#include <utility>

#include <boost/algorithm/string/regex.hpp>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {

class word_count : public paracel::paralg {

public:
  word_count(paracel::Comm comm, std::string hosts_dct_str, 
  	std::string input, std::string output, 
	int k, int limit_s = 3, bool ssp_switch = true) : 
	paracel::paralg(hosts_dct_str, comm, output, rounds = 1, limit_s, ssp_switch),
	topk(k) {}

  virtual ~word_count();
  
  std::vector<std::string> parser(const std::string & line) {
    std::vector<std::string> wl, rl;
    boost::algorithm::split_regex(wl, line, regex("[^-a-zA-Z0-9_]"));
    for(int i = 0; i < wl.size(); ++i) {
      if(wl[i]) {
        rl.push_back(wl[i]);
      }
    }
    return rl;
  }

  virtual void solve() {
    //std::string delimiter = "[^-a-zA-Z0-9_]"; 
    paracel_register_bupdate("/mfs/user/wuhong/paracel/alg/wc/update.so", "wc_updater");
    auto lines = paracel_load(input);
    sync();
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        if(is_cached(word)) {
	  paracel_bupdate(word, 1);
	} else {
	  if(paracel_contains(word)) {
	    paracel_bupdate(word, 1); 
	  } else {
	    paracel_write(word, 1);
	  }
	}
      } // word_lst
    } // lines
    sync();
    result = paracel_read_topk(topk);
  }

private:
  int topk = 10;
  std::vector<std::pair<std::string, size_t> > result;
};

} // namespace paracel

#endif
