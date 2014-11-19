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

#ifndef FILE_327f02ad_6252_7bd8_bcf0_be981ff89928_HPP
#define FILE_327f02ad_6252_7bd8_bcf0_be981ff89928_HPP

#include <cmath>
#include <cfloat>
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <eigen3/Eigen/Sparse>
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

namespace paracel {

class recommendation_decision_tree : public paralg {

 public:
  recommendation_decision_tree(paracel::Comm comm, std::string hosts_dct_str,
                               std::string input1, std::string input2, std::string output, 
                               int height) : 
    paracel::paralg(hosts_dct_str, comm, output, 1, 0, false), input1(input1), input2(input2), height(height) {
    bigraph_u = new bigraph_continuous(input1);
    bigraph_i = new bigraph_continuous(input2); 
    for(int i = 0; i < bigraph_u->v(); ++i) {
      users.insert(i);
    }
  }

  virtual ~recommendation_decision_tree() {
    delete bigraph_u;
    delete bigraph_i;
  }

  double foo(double s2, double s, int n) {
    if(n == 0) { return 0; }
    return s2 - s * s / n;
  }

  // build a decision tree under node t representing user set S_t
  void generate_decision_tree(const std::unordered_set<int> & S_t) {
    
    int item_sz = bigraph_i->v();
    std::vector<double> err(item_sz, 0.), sum_all(item_sz, 0.), sum2_all(item_sz, 0.);
    std::vector<int> nall(item_sz, 0);
    int complexity = 0;

    for(int i = 0; i < item_sz; ++i) {
      for(auto & pair : bigraph_i->adjacent(i)) {
        int u = pair.first;
        double r = pair.second;
        if(!S_t.count(u)) { continue; }
        sum_all[i] += r;
        sum2_all[i] += r * r;
        nall[i] += 1;
      }
    }

    // splitting item selection:
    for(int i = 0; i < item_sz; ++i) {
      /*
      bool flag = false;
      for(size_t indx = 0; indx < result.size(); ++indx) {
        if(result[indx] == i) {
          flag = true;
          break;
        }
      }
      if(flag) { continue; }
      */
      std::vector<double> sum_tL(item_sz, 0.), sum2_tL(item_sz, 0.);
      std::vector<double> sum_tH(item_sz, 0.), sum2_tH(item_sz, 0.);
      std::vector<int> n_tL(item_sz, 0), n_tH(item_sz, 0);

      for(auto & pair : bigraph_i->adjacent(i)) {
        bool flag_L = false, flag_H = false;
        int u = pair.first; double r = pair.second;
        if(!S_t.count(u)) { continue; }
        if(r >= 4) {
          flag_L = true;
        } else {
          flag_H = true;
        }
        for(auto & pair_inside: bigraph_u->adjacent(u)) {
          //complexity += 1;
          int j = pair_inside.first; double r_uj = pair_inside.second;
          if(flag_L) {
            sum_tL[j] += r_uj;
            sum2_tL[j] += r_uj * r_uj;
            n_tL[j] += 1;
          }
          if(flag_H) {
            sum_tH[j] += r_uj;
            sum2_tH[j] += r_uj * r_uj;
            n_tH[j] += 1;
          }
        } // pair_inside 
      } // pair
      // calculate err[i]
      for(int ii = 0; ii < item_sz; ++ii) {
        double e2_tL = foo(sum2_tL[ii], sum_tL[ii], n_tL[ii]);
        double e2_tH = foo(sum2_tH[ii], sum_tH[ii], n_tH[ii]);
        double e2_tU = foo((sum2_all[ii] - sum2_tL[ii] - sum2_tH[ii]), 
                           ((sum_all[ii] - sum_tL[ii] - sum_tH[ii])), 
                           (nall[ii] - n_tL[ii] - n_tH[ii]));
        err[i] += e2_tL + e2_tH + e2_tU;
      }
      //std::cout << i << " 's err is: " << err[i] << std::endl;
      //std::cout << "complexity: " << complexity << std::endl;
    } // i
    
    std::cout << err.size() << std::endl;
    auto result_it = std::minmax_element(err.begin(), err.end());
    int partition_id = result_it.first - err.begin();
    std::cout << partition_id << " with min error: " << err[partition_id] 
        << " and max error: " << err[result_it.second - err.begin()] << std::endl;

/*
    // creation of t's subtrees
    if(level != height - 1) {
      std::unordered_set<std::string> L, H, U;
      for(auto & u : S_t) {
        if(!graph_u.adjacent(u).count(partition_id)) {
          U.insert(u);
        } else {
          if(graph_u.adjacent(u)[partition_id] >= 4.) {
            L.insert(u);
          } else {
            H.insert(u);
          }
        }
      }
      for(auto & debug : L) {
        std::cout << debug << std::endl;
      }
      std::cout << "---" << std::endl;
      for(auto & debug : H) {
        std::cout << debug << std::endl;
      }
      std::cout << "---" << std::endl;
      for(auto & debug : U) {
        std::cout << debug << std::endl;
      }
      std::cout << "---" << std::endl;
      // recursively call
      generate_decision_tree(L);
      generate_decision_tree(H);
      generate_decision_tree(U);
    }
*/
  }


  void solve() {
    generate_decision_tree(users);
    /*
    for(auto & r : result) {
      std::cout << r << std::endl;
    }
    */
  }

 private:
  std::string input1, input2, output;
  int height = 0;
  int level = 0;
  paracel::bigraph_continuous *bigraph_u;
  paracel::bigraph_continuous *bigraph_i;
  std::unordered_set<int> users;
  std::vector<std::string> result;
 private:
  Eigen::SparseMatrix<double, Eigen::RowMajor> A;
  std::unordered_map<size_t, std::string> row_map;
}; // class recommendation_decision_tree

} // namespace paracel

#endif
