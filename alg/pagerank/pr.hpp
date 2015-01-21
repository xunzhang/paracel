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

#ifndef FILE_c8a6be00_163f_6ae8_c447_47f0d8483425_HPP
#define FILE_c8a6be00_163f_6ae8_c447_47f0d8483425_HPP

#include <string>
#include <unordered_map>
#include "ps.hpp"
#include "load.hpp"
#include "graph.hpp"

namespace paracel {

class pagerank : public paracel::paralg {
 
 public:
  pagerank(paracel::Comm comm, 
           std::string hosts_dct_str,
           std::string _input,
           std::string _output,
           int _rounds = 1,
           double df = 0.85,
           int limit_s = 0) : 
    paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, true),
    input(_input),
    rounds(_rounds), 
    damping_factor(df) {}

  virtual ~pagerank() {}

  void init_paras() {
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(local_parser);
    if(get_worker_id() == 0) std::cout << "bug0" << std::endl;
    paracel_load_as_graph(local_graph, input, f_parser, "fmap");

    auto cnt_lambda = [&] (const std::string & a,
                           const std::string & b,
                           double c) {
      if(!kvmap.count(a)) {
        kvmap[a] = 1.;
      } else {
        kvmap[a] += 1.;
      }
    };
    local_graph.traverse(cnt_lambda);
    if(get_worker_id() == 0) std::cout << "bug1" << std::endl;
    
    // make sure input data is clean which means there are no same pieces
    // generate kv + local combine
    auto kvinit_lambda = [&] (const std::string & a,
                              const std::string & b,
                              double c) {
      klstmap[b].push_back(std::make_pair(a, kvmap[a]));
    };
    local_graph.traverse(kvinit_lambda);
    if(get_worker_id() == 0) std::cout << "bug2" << std::endl;

    // init push to construct global connect info
    paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/libpr_update.so",
                             "init_updater");
    for(auto & kv : klstmap) {
      paracel_bupdate(kv.first + "_links", kv.second);
      //paracel_update_default(kv.first + "_links", kv.second);
    }

    // read connect info only once
    klstmap.clear();
    for(auto & kv : kvmap) {
      // notice: limit memory here
      klstmap[kv.first] = paracel_read<
          std::vector<std::pair<std::string, double> > 
          >(kv.first + "_links");
    }

    // reuse kvmap to store pr
    // init pr with 1./total_node_sz
    auto worker_comm = get_comm();
    long node_sz = kvmap.size();
    worker_comm.allreduce(node_sz);
    double init_val = 1. / node_sz;
    for(auto & kv : kvmap) {
      kvmap[kv.first] = init_val; 
      paracel_write(kv.first + "_pr", init_val);
    }
  }

  void learning() {
    // first read
    std::unordered_map<std::string, double> kvmap_stale;
    for(auto & kv : klstmap) {
      for(auto & k : kv.second) {
        if(!kvmap_stale.count(k.first)) {
          kvmap_stale[k.first] = paracel_read<double>(k.first + "_pr");
        }
      }
    }

    for(int rd = 0; rd < rounds; ++rd) {
      if(get_worker_id() == 0) std::cout << rd << std::endl;
      // pull
      for(auto & kv : kvmap_stale) {
        kvmap_stale[kv.first] = paracel_read<double>(kv.first + "_pr");
      }

      // map
      for(auto & kv : klstmap) {
        double sigma = 0.;
        for(auto & item : kv.second) {
          sigma += (kvmap_stale[item.first] / item.second);
        }
        kvmap[kv.first] = (1. - damping_factor) + damping_factor * sigma;
      }
      sync();

      // reduce
      for(auto & kv : kvmap) {
        paracel_write(kv.first + "_pr", kv.second);
      }
      sync();
    }
    // last pull all
    auto kvmap_tmp = paracel_read_special<double>(
        "/mfs/user/wuhong/paracel/build/lib/libpr_update.so",
        "pr_filter");
    auto tear_lambda = [] (const std::string & str) {
      auto pos = str.find('_');
      return str.substr(0, pos);
    };
    for(auto & kv : kvmap_tmp) {
      std::string tmp = tear_lambda(kv.first);
      kvmap[tmp] = kv.second;
    }
  }

  void dump_result() {
    if(get_worker_id() == 0) {
      paracel_dump_dict(kvmap, "pagerank_");
    }
  }

  void solve() {
    init_paras();
    sync();
    learning();
  }

 private:
  std::string input;
  int rounds;
  double damping_factor;
  paracel::digraph<std::string> local_graph;
  std::unordered_map<std::string, double> kvmap;
  std::unordered_map<
      std::string, 
      std::vector<std::pair<std::string, double> > 
          > klstmap;
}; // class pagerank

} // namespace paracel

#endif
