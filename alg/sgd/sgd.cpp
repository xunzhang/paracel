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
#include <iostream>
#include "sgd.hpp"
#include "ps.hpp"
#include "utils.hpp"

using std::vector;
using std::string;

namespace paracel {

sgd::sgd(paracel::Comm comm, string hosts_dct_str, 
	string _input, string output,
	size_t _rounds, double _alpha, double _beta, bool _debug) :
		paracel::paralg(hosts_dct_str, comm, output, _rounds),
		input(_input),
		worker_id(comm.get_rank()),
		rounds(_rounds), 
		alpha(_alpha),
		beta(_beta),
		debug(_debug) {}

sgd::~sgd() {}

// logistic regression hypothesis function
// e .** (v .dot theta) / (1 + e .** (v .dot theta))
double sgd::lg_hypothesis(const vector<double> & v) {
  double dp = paracel::dot_product(v, theta);
  double temp = 1. / (1. + exp(dp));
  return exp(dp) * temp;
}

// set samples and labels member
void sgd::local_parser(const vector<string> & linelst, const char sep = ',') {
  samples.resize(0);
  labels.resize(0);
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
  double coff2 = 2. * beta * alpha;
  double tmp = 1. / get_worker_size();
  std::cout << "wgt: " << tmp << std::endl;
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    int cnt = 0;
    std::random_shuffle(idx.begin(), idx.end()); 
    // traverse data
    for(auto id : idx) {
      cnt += 1;
      //if((cnt == 0) || (cnt % 500 == 0)) { theta = paracel_read<vector<double> >("theta"); }
      theta = paracel_read<vector<double> >("theta"); 
      double grad = labels[id] - lg_hypothesis(samples[id]); 
      double coff1 = alpha * grad;
      vector<double> delta; 
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[id][i] - coff2 * theta[i];
        delta.push_back(t);
      }
      for(int i = 0; i < data_dim; ++i) {
        //theta[i] += delta[i];
	delta[i] *= tmp; // average delta
      }
      paracel_update("theta", delta); // update with delta
      if(debug) {
        loss_error.push_back(calc_loss());
      }
    } // end traverse
    //sync();
    std::cout << "end of rd: " << rd << std::endl;
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
  //print(theta);
  //dump_result();
}

double sgd::calc_loss() {
  double loss = 0.;
  for(int i = 0; i < samples.size(); ++i) {
    double j = lg_hypothesis(samples[i]);
    loss += j * j;
  }
  auto worker_comm = get_comm();
  worker_comm.allreduce(loss);
  int sz = samples.size();
  worker_comm.allreduce(sz);
  return loss / sz;
}

void sgd::dump_result() {
  if(get_worker_id() == 0) {
    dump_vector(theta, "lg_theta_", "|");
    dump_vector(loss_error, "lg_loss_error_", "\n");
  }
}

void sgd::print(const vector<double> & vl) {
  for(auto & v : vl) {
    std::cout << v << "|";
  }
  std::cout << std::endl;
}

void sgd::predict(const std::string & pred_fn) {
  auto lines = paracel_load(input);
  local_parser(lines); // re-init samples, labels
  std::cout << "mean loss" << calc_loss() << std::endl;
}

} // namespace paracel
