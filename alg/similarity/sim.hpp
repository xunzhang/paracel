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
 * For fset fmt usage
 */

#ifndef FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP
#define FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP

#include <assert.h>
#include <cmath>
#include <cfloat>
#include <string>
#include <utility>
#include <algorithm>
#include <unordered_map>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {

class similarity : public paracel::paralg {

 public:

  similarity(paracel::Comm comm, 
             std::string hosts_dct_str,
             std::string _input,
             std::string _output,
             int k = 20,
             std::string _method = "default") :
      paracel::paralg(hosts_dct_str, comm, _output),
      input(_input),
      output(_output),
      learning_method(_method),
      ktop(k) {}

  virtual ~similarity() {}

 private:

  void local_parser(std::unordered_map<std::string, std::vector<double> > & var,
                    const std::vector<std::string> & linelst, 
                    const char sep = ',') {
    for(auto & line : linelst) {
      std::vector<double> tmp;
      auto v = paracel::str_split(line, sep);
      for(size_t i = 1; i < v.size(); ++i) {
        tmp.push_back(std::stod(v[i]));
      }
      var[v[0]] = tmp;
    }
  }

  void normalize(std::unordered_map<std::string, std::vector<double> > & var) {
    for(auto & kv : var) {
      double square_sum = 0.;
      // calc square sum
      for(size_t i = 0; i < kv.second.size(); ++i) {
        square_sum += std::pow(kv.second[i], 2);
      }
      // divide
      for(auto it = kv.second.begin(); it != kv.second.end(); ++it) {
        *it = *it / std::sqrt(square_sum);
      }
    }
  }

  void init_paras() {
    for(auto & kv : item_vects) {
      paracel_write(kv.first, kv.second); // push vector
      item_bag.push_back(kv.first);
    }
    std::string key = "item_bag_" + std::to_string(get_worker_id());
    paracel_write(key, item_bag); // push iids
  }

  unsigned long cvt2num(const std::string & s) {
    return std::stoul(s);
  }

  void select_top() {
    auto comp = [] (std::pair<std::string, double> a,
                    std::pair<std::string, double> b) {
      return std::get<1>(a) > std::get<1>(b);
    };
    for(auto & iv : item_vects) {
      std::sort(result[iv.first].begin(), result[iv.first].end(), comp);
      assert((size_t)ktop <= result[iv.first].size());
      result[iv.first].resize(ktop);
    }
  }

  void local_learning(const std::unordered_map<std::string, std::vector<double> > & var) {
    for(auto & iv : item_vects) {
      for(auto & jv : var) {
        if(iv.first != jv.first) {
          double sim = paracel::dot_product(iv.second, jv.second);
          result[iv.first].push_back(std::make_pair(jv.first, sim));
        }
      } // for jv
    } // for iv
  }

  void learning() {
    local_learning(all_item_vects);
    std::cout << "learing almost finished" << std::endl;
    sync();
    select_top();
    std::cout << "topk got" << std::endl;
    sync();
  }

  void mls_learning() {

    // learn with local item vectors
    local_learning(item_vects);
    sync();	

    // calc similarity with items in other proces 
    for(int node_id = 0; node_id < (int)get_worker_size(); ++node_id) {
      if(node_id == (int)get_worker_id()) continue;
      auto id_bag = paracel_read<std::vector<std::string> >(
          "item_bag_" + 
          std::to_string(node_id)
          );
      for(auto & iv : item_vects) {
        for(auto & iid : id_bag) {
          auto jv = paracel_read<std::vector<double> >(iid);
          double sim = paracel::dot_product(iv.second, jv);
          result[iv.first].push_back(std::make_pair(iid, sim));
        } // id_bag
      } // for iv
    } // bcast_ring 
    sync();

    // get ktop
    select_top();
    sync();
  }

 public:

  virtual void solve() {

    auto lines = paracel_load(input);
    local_parser(item_vects, lines);
    std::cout << "parser done" << std::endl;

    if(learning_method == "default") {
      auto all_lines = paracel_loadall(input);
      local_parser(all_item_vects, all_lines);
      std::cout << "loadall done" << std::endl;
      normalize(item_vects);
      normalize(all_item_vects);
      std::cout << "normalize done" << std::endl;
      sync();
      learning();
    } else if(learning_method == "limit_storage") {
      normalize(item_vects); // normalize here to reduce calculation
      init_paras();
      sync();
      mls_learning();
    } else {}

  }

  void dump_result() {
    paracel_dump_dict(result);
  }


 private:
  std::string input;
  std::string output;
  std::string learning_method;
  int ktop = 20;
  std::unordered_map<std::string, std::vector<double> > item_vects;
  std::unordered_map<std::string, std::vector<double> > all_item_vects; // for default matmul usage
  std::vector<std::string> item_bag;
  std::unordered_map<std::string, std::vector<std::pair<std::string, double> > >result;
};

} // namespace paracel

#endif
