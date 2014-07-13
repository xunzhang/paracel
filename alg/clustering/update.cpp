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

#include <vector>
#include <eigen3/Eigen/Dense>
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;

extern "C" {
  extern paracel::update_result local_update_clusters;
}

vector<vector<double> > local_update_clusters_stl(const vector<vector<double> > & a ,
                                                  const vector<vector<double> > & b) {
  size_t kclusters = a.size();
  size_t dims = a[0].size();
  vector<vector<double> > r(kclusters);
  for(size_t k = 0; k < kclusters; ++k) {
    r[k].resize(dims);
  }
  for(size_t k = 0; k < kclusters; ++k) {
    for(size_t dim = 0; dim < dims; ++dim) {
      r[k][dim] = a[k][dim] + b[k][dim];
    }
  }
  return r;
}

// TODO
/*
Eigen::MatrixXd local_update_clusters_eigen(const Eigen::MatrixXd & a,
                                            const Eigen::MatrixXd & b) {}
*/

 paracel::update_result local_update_clusters = paracel::update_proxy(local_update_clusters_stl);
 //paracel::update_result local_update_clusters = paracel::update_proxy(local_update_clusters_eigen);
