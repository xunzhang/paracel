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
#include <set>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <unordered_map>

#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {

class word_count : public paracel::paralg {

public:
  word_count(paracel::Comm comm, std::string hosts_dct_str, 
  	std::string _input, std::string _output, std::string method = "normal",
	int k = 10, int limit_s = 3, bool ssp_switch = true) : 
	paracel::paralg(hosts_dct_str, comm, _output, 1, limit_s, ssp_switch),
	input(_input),
	learning_method(method),
	topk(k) {}

  virtual ~word_count() {}
  
  std::vector<std::string> parser(const std::string & line) {
    std::vector<std::string> wl, rl;
    boost::algorithm::split_regex(wl, line, boost::regex("[^-a-zA-Z0-9_]"));
    for(size_t i = 0; i < wl.size(); ++i) {
      if(wl[i] != "") {
        rl.push_back(wl[i]);
      }
    }
    return rl;
  }
  
  void normal_learning(const std::vector<std::string> & lines) {
    paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/libwc_update.so", "wc_updater");
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
      iter_commit();
    } // lines
    sync();
    paracel_read_topk(topk, result);
  }

  void optimized_learning(const std::vector<std::string> & lines) {
    paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/libwc_update.so", "wc_updater");
    /*
    // init para
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        paracel_write(word, 0); 
      } // word_lst
    } // lines
    */
    // init para
    std::unordered_map<std::string, int> tmp;
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        tmp[word] = 0;
      }
    }
    paracel_write_multi(tmp);
    sync();
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        paracel_bupdate(word, 1);
      }
    }
    iter_commit();
    sync();
    paracel_read_topk(topk, result);
  }

  virtual void solve() {
    auto lines = paracel_load(input);
    sync();
    if(learning_method == "normal") {
      set_total_iters(lines.size());
      normal_learning(lines);
    } else if(learning_method == "optimized") {
      set_total_iters(1);
      optimized_learning(lines);
    } else {
      std::cout << "learning method not supported." << std::endl;
      return;
    }
    sync();
  }

  void print() {
    if(get_worker_id() == 0) {
      for(auto & pr : result) {
        std::cout << std::get<0>(pr) << " : " << std::get<1>(pr) << std::endl;
      }
    }
  }

private:
  std::string input;
  std::string learning_method;
  int topk = 10;
  std::vector<std::pair<std::string, int> > result;
};

} // namespace paracel

#endif
