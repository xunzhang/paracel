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

#ifndef FILE_a3d185a2_149d_fe7f_90a8_fbbbc7a8209a_H 
#define FILE_a3d185a2_149d_fe7f_90a8_fbbbc7a8209a_H

#include <string>
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

namespace paracel {

auto local_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
};

class content_base_recommendation: public paracel::paralg {

public:
  content_base_recommendation(paracel::Comm comm, string hosts_dct_str,
                              string _input, string _output, int _rounds = 1,
                              double _alpha = 0.005, double _beta = 0.01,
                              int limit_s = 3, bool ssp_switch = true) :
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, ssp_switch),
      input(_input),
      output(_output),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta) {}
  
  virtual ~content_base_recommendation() {}

  virtual void solve() {
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(rating_graph, input, f_parser "fsmap");
  }

};

private:
  string input, output;
  int fac_dim;
  int rounds;
  double alpha, beta;
  
  paracel::bigraph<std::string> rating_graph;
}

#endif
