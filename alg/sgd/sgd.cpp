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

#include <math.h>
#include <string>
#include <algorithm>
#include "sgd.hpp"
#include "ps.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

sgd::sgd(paracel::Comm comm, string hosts_dct_str, 
	string _input, string output, size_t nworker,
	size_t _rounds, double _alpha, double _beta) :
		paracel::paralg(hosts_dct_str, comm, output, _rounds),
		input(_input),
		worker_id(comm.get_rank()),
		rounds(_rounds), 
		alpha(_alpha),
		beta(_beta) {}

sgd::~sgd() {}

double sgd::loss_func_grad(const vector<double> & v) {
  double dp = paracel::dot_product(v, theta);
  double temp = 1. / (1. + exp(dp));
  return exp(dp) * temp;
}

// set samples and labels member
void sgd::local_parser(const vector<string> & linelst, const char sep = ',') {
  for(auto & line : linelst) {
    vector<double> tmp;
    double label;
    auto linev = paracel::str_split(line, sep); 
    tmp.push_back(1.);
    for(int i = 0; i < linev.size() - 1; ++i) {
      tmp.push_back(std::stod(linev[i]));
    }
    samples.push_back(tmp);
    labels.push_back(std::stod(linev[linev.size() - 1]));
  }
} 

void sgs::learning() {
  int data_sz = samples.size(), data_dim = samples[0].size();
  
  theta = paracel::random_double_list(data_dim); 
  paracel_wrire("theta", theta); // push
  
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_update();
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    // traverse data
    for(auto id : idx) {
      theta = paracel_read<vector<double> >("theta"); 
      double grad = labels[id] - sgd::loss_func_grad(samples[id]); 
      vector<double> delta; 
      for(int i = 0; i < data_dim; ++i) {
        double t = alpha * grad * samples[id][i] - 2. * beta * alpha * theta[i];
        delta.push_back(t);
      }
      paracel_update("theta", delta); // update with delta
      for(int i = 0; i < data_dim; ++i) {
        theta[i] += delta[i];
      }
    } // end traverse
  } // end rounds
  sync();
  theta = paracel_read<vector<double> >("theta"); // last pull
  sync();
}

void sgd::solve() {
  auto lines = paracel_load(input);
  local_parser(lines); // init data
  sync();
  learning();
}

void sgd::dump_result() {}

} // namespace alg
} // namespace paracel
