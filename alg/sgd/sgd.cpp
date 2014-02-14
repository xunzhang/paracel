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

#include <iostream>

namespace paracel {

sgd::sgd(paracel::Comm comm, string hosts_dct_str, 
	string _input, string output,
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
/*
  for(auto & v : samples[0]) {
    std::cout << "dim: " << v << std::endl;
  }
  std::cout << "label: " << labels[0] << std::endl;
*/
} 

void sgd::learning() {
  int data_sz = samples.size(), data_dim = samples[0].size();
  
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // push
  
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_update("/mfs/user/wuhong/paracel/alg/sgd/update.so", "sgd_theta_update");
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    // traverse data
    for(auto id : idx) {
      theta = paracel_read<vector<double> >("theta"); 
      double grad = labels[id] - loss_func_grad(samples[id]); 
      vector<double> delta; 
      for(int i = 0; i < data_dim; ++i) {
        double t = alpha * grad * samples[id][i] - 2. * beta * alpha * theta[i];
        delta.push_back(t);
      }
      paracel_update("theta", delta); // update with delta
      for(int k = 0; k < 100000; ++k) {
        int p = k + 1;
      }
      for(int i = 0; i < data_dim; ++i) {
        theta[i] += delta[i];
      }
    } // end traverse
  } // end rounds
  sync();
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void sgd::solve() {
  auto lines = paracel_load(input);
  local_parser(lines); // init data
  sync();
  learning();
  sync();
  print(theta);
}
double sgd::calc_loss() {}
void sgd::dump_result() {}
void sgd::print(const vector<double> & vl) {
  for(auto & v : vl) {
    std::cout << v << "|";
  }
  std::cout << std::endl;
}

} // namespace paracel
