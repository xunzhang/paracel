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

#ifndef PARACEL_SGD_HPP
#define PARACEL_SGD_HPP

#include <string>
#include <vector>
#include "ps.hpp"
#include "utils.hpp"

using namespace std;

namespace paracel {

class sgd : public paracel::paralg {

public:
  sgd(paracel::Comm, string, string, string, size_t = 1, double = 0.002, double = 0.1);
  virtual ~sgd();
  
  double loss_func_grad(const vector<double> &);
  void local_parser(const vector<string> &, const char);
  
  virtual void learning();
  virtual void solve();
  
  double calc_loss();
  void dump_result();
  void print(const vector<double> &);

private:
  size_t worker_id;
  size_t rounds;
  double alpha;
  double beta;
  string input;
  vector<vector<double> > samples;
  vector<double> labels;
  vector<double> theta;

}; 

} // namespace paracel

#endif
