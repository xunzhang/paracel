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

#ifndef FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP
#define FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP
#include <set>
#include <queue>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

namespace paracel {

struct local_min_heap_cmp {
  inline bool operator() (const std::pair<paracel::str_type, int> & l,
  const std::pair<paracel::str_type, int> & r) {
    return l.second > r.second;
  }
};

class word_count : public paracel::paralg {

 public:
  word_count(paracel::Comm comm, std::string hosts_dct_str, 
             std::string _input, std::string _output, std::string method = "normal",
             int k = 10) : 
      paracel::paralg(hosts_dct_str, comm, _output),
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
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        paracel_bupdate(word, 1, "/mfs/user/wuhong/paracel/local/lib/libwc_update.so", "wc_updater");
      } // word_lst
    } // lines
    sync();
    paracel_read_topk(topk, result);
  }

  void noraml_learning_with_combine(const std::vector<std::string> & lines) {
    std::unordered_map<std::string, int> tmp;
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        if(tmp.count(word)) {
          tmp[word] += 1;
        } else {
          tmp[word] = 1;
        }
      }
    }
    for(auto & wc : tmp) {
      paracel_bupdate(wc.first, wc.second);
    }
    sync();
    paracel_read_topk(topk, result);
  }

  void optimized_learning(const std::vector<std::string> & lines) {
    std::unordered_map<std::string, int> tmp;
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        if(tmp.count(word)) {
          tmp[word] += 1;
        } else {
          tmp[word] = 1;
        }
      }
    }
    int dct_sz = 10;
    std::vector<std::unordered_map<std::string, int> > local_vd(dct_sz);
    paracel::hash_type<std::string> hfunc;
    for(auto & kv : tmp) {
      auto indx = hfunc(kv.first) % dct_sz;
      local_vd[indx][kv.first] = kv.second;
    }
    sync();
    for(int k = 0; k < dct_sz; ++k) {
      paracel_bupdate("key_" + std::to_string(k),
                      local_vd[k],
                      "/mfs/user/wuhong/paracel/local/lib/libwc_update.so",
                      "wc_updater2");
    }
    std::cout << "done" << std::endl;
    sync();
    // get topk
    using min_heap = std::priority_queue<std::pair<std::string, int>,
          std::vector<std::pair<std::string, int> >,
          local_min_heap_cmp>;
    
    min_heap tmplst;
    auto handler = [&] (const std::unordered_map<std::string, std::unordered_map<std::string, int> > & d) {
      for(auto & kv : d) {
        for(auto & kkv : kv.second) {
          auto node = paracel::heap_node<paracel::str_type, int>(kkv.first, kkv.second);
          tmplst.push(node.val);
          if((int)tmplst.size() > topk) {
            tmplst.pop();
          }
        }
      }
    };
    paracel_read_special_handle<std::unordered_map<std::string, int> >("/mfs/user/wuhong/paracel/local/lib/libwc_update.so", 
                                                                       "wc_filter", 
                                                                       handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first, tmplst.top().second));
      tmplst.pop();
    }
    std::reverse(result.begin(), result.end());
  }

  virtual void solve() {
    auto lines = paracel_load(input);
    std::cout << "load done" << std::endl;
    sync();
    if(learning_method == "normal") {
      normal_learning(lines);
    } else if(learning_method == "normal_with_combine") {
      noraml_learning_with_combine(lines);
    } else if(learning_method == "optimized") {
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
