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

#ifndef PARACEL_LOGISTIC_REGRESSION_HPP
#define PARACEL_LOGISTIC_REGRESSION_HPP

#include <string>
#include <vector>
#include "ps.hpp"
#include "utils.hpp"

using namespace std;

namespace paracel {

class logistic_regression: public paracel::paralg {

public:
  logistic_regression(paracel::Comm, string, string, string, string = "ipm", int = 1, double = 0.002, double = 0.1, bool = false);
  virtual ~logistic_regression();
  
  double lg_hypothesis(const vector<double> &);
  void local_parser(const vector<string> &, const char);
  
  void dgd_learning(); // distributed gradient descent learning
  void ipm_learning(); // iterative parameter mixtures learning
  void agd_learning(); // asynchronous gradient descent learning

  virtual void solve();
  
  double calc_loss();
  void dump_result();
  void print(const vector<double> &);
  void predict(const std::string &);

private:
  int worker_id;
  int rounds;
  double alpha;
  double beta;
  string input;
  vector<vector<double> > samples;
  vector<double> labels;
  vector<double> theta;
  bool debug = false;
  vector<double> loss_error;
  std::string learning_method;
}; 

} // namespace paracel

#endif
