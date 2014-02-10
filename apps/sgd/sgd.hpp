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
#include "ps.hpp"
#include "utils.hpp"

using namespace std;

namespace alg {

class sgd : public paracel::paralg {
public:
  sgd(paracel::Comm, string, string, size_t, size_t, size_t = 1, double = 0.002, double = 0.1);
  virtual ~sgd();
  virtual solve();
private:
  size_t worker_id;
  size_t rounds;
  double alpha;
  double beta;
  string input;
}; 

} // namespace alg 

#endif
