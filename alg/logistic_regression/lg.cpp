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
#include "lg.hpp"
#include "ps.hpp"
#include "utils.hpp"

using std::vector;
using std::string;

namespace paracel {

logistic_regression::logistic_regression(paracel::Comm comm, string hosts_dct_str, 
					string _input, string output, string method,
					int _rounds, double _alpha, double _beta, bool _debug,
					int limit_s, bool ssp_switch) :
	paracel::paralg(hosts_dct_str, comm, output, _rounds),
	input(_input),
	learning_method(method),
	worker_id(comm.get_rank()),
	rounds(_rounds), 
	alpha(_alpha),
	beta(_beta),
	debug(_debug) {}

logistic_regression::~logistic_regression() {}

// logistic regression hypothesis function
// e .** (v .dot theta) / (1 + e .** (v .dot theta))
double logistic_regression::lg_hypothesis(const vector<double> & v) {
  double dp = paracel::dot_product(v, theta);
  return 1. / (1. + exp(-dp));
}

// set samples and labels member
void logistic_regression::local_parser(const vector<string> & linelst, const char sep = ',') {
  samples.resize(0);
  labels.resize(0);
  for(auto & line : linelst) {
    vector<double> tmp;
    //double label;
    auto linev = paracel::str_split(line, sep); 
    tmp.push_back(1.);
    for(size_t i = 0; i < linev.size() - 1; ++i) {
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

void logistic_regression::dgd_learning() {
  int data_sz = samples.size(), data_dim = samples[0].size();
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // init push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/liblg_update.so", 
  			"lg_theta_update");
  double coff2 = 2. * beta * alpha;
  vector<double> delta(data_dim); 
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    for(int i = 0; i < data_dim; ++i) {
      delta[i] = 0.;
    }
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    // traverse data
    for(auto sample_id : idx) {
      double grad = labels[sample_id] - lg_hypothesis(samples[sample_id]); 
      double coff1 = alpha * grad;
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
	delta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
    } // traverse
    sync(); // sync for map
    paracel_bupdate("theta", delta); // update with delta
    sync(); // sync for reduce
    std::cout << "worker" << get_worker_id() << " at the end of rd" << rd << std::endl;
  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::ipm_learning() {
  int data_sz = samples.size(), data_dim = samples[0].size();
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // init push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/liblg_update.so", 
  			"lg_theta_update");
  double coff2 = 2. * beta * alpha;
  double wgt = 1. / get_worker_size();
  vector<double> delta(data_dim);
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    vector<double> theta_old(theta);
    // traverse data
    for(auto sample_id : idx) {
      for(int i = 0; i < data_dim; ++i) {
        double coff1 = alpha * (labels[sample_id] - lg_hypothesis(samples[sample_id])); 
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
	theta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
    } // traverse
    for(int i = 0; i < data_dim; ++i) {
      delta[i] = wgt * (theta[i] - theta_old[i]);
    }
    sync(); // sync for map
    paracel_bupdate("theta", delta); // update with delta
    sync(); // sync for reduce
    std::cout << "worker" << get_worker_id() << " at the end of rd" << rd << std::endl;
  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::agd_learning() {
  int data_sz = samples.size(), data_dim = samples[0].size();
  int cnt = 0, read_batch = data_sz / 100, update_batch = data_sz / 100;
  if(read_batch == 0) { read_batch = 10; }
  if(update_batch == 0) { update_batch = 10; }
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // init push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/liblg_update.so", 
  			"lg_theta_update");
  double coff2 = 2. * beta * alpha;
  vector<double> delta(data_dim);
  // main loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    vector<double> theta_old(theta);
    // traverse data
    cnt = 0;
    for(auto sample_id : idx) {
      if( (cnt % read_batch == 0) || (cnt == (int)idx.size() - 1) ) { 
        theta = paracel_read<vector<double> >("theta"); 
	theta_old = theta;
      }
      for(int i = 0; i < data_dim; ++i) {
        double coff1 = alpha * (labels[sample_id] - lg_hypothesis(samples[sample_id])); 
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
	theta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
      if( (cnt % update_batch == 0) || (cnt == (int)idx.size() - 1) ) {
        for(int i = 0; i < data_dim; ++i) {
          delta[i] = theta[i] - theta_old[i];
        }
	paracel_bupdate("theta", delta);
      }
      cnt += 1;
    } // traverse
    sync();
    std::cout << "worker" << get_worker_id() << " at the end of rd" << rd << std::endl;
  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::solve() {
  auto lines = paracel_load(input);
  local_parser(lines); // init data
  sync();
  if(learning_method == "dgd") {
    dgd_learning();
  } else if(learning_method == "ipm") {
    ipm_learning();
  } else if(learning_method == "agd") {
    agd_learning();
  } else {
    std::cout << "learning method not supported." << std::endl;
    return;
  }
  sync();
  //print(theta);
}

double logistic_regression::calc_loss() {
  double loss = 0.;
  for(size_t i = 0; i < samples.size(); ++i) {
    double j = lg_hypothesis(samples[i]);
    loss += j * j;
  }
  auto worker_comm = get_comm();
  worker_comm.allreduce(loss);
  int sz = samples.size();
  worker_comm.allreduce(sz);
  return loss / sz;
}

void logistic_regression::dump_result() {
  if(get_worker_id() == 0) {
    paracel_dump_vector(theta, "lg_theta_", "|");
    paracel_dump_vector(loss_error, "lg_loss_error_", "\n");
  }
}

void logistic_regression::print(const vector<double> & vl) {
  for(auto & v : vl) {
    std::cout << v << "|";
  }
  std::cout << std::endl;
}

void logistic_regression::predict(const std::string & pred_fn) {
  auto lines = paracel_load(input);
  local_parser(lines); // re-init samples, labels
  std::cout << "mean loss" << calc_loss() << std::endl;
}

} // namespace paracel
