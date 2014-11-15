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
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

namespace paracel {

class recommendation_decision_tree : public paralg {

 public:
  recommendation_decision_tree(paracel::Comm comm, std::string hosts_dct_str,
                               std::string input, std::string output, int height) : 
    paracel::paralg(hosts_dct_str, comm, output, 1, 0, false), input(input), height(height) {}

  virtual ~recommendation_decision_tree() {}

  double foo(double s2, double s, int n) {
    if(n == 0) { return 0; }
    return s2 - s * s / n;
  }

  // build a decision tree under node t representing user set S_t
  void generate_decision_tree(const std::unordered_set<std::string> & S_t) {
    std::unordered_map<std::string, double> err;
    std::unordered_map<std::string, double> sum_all, sum2_all;
    std::unordered_map<std::string, int> nall;
    //int complexity = 0;
    for(auto & i : items) {
      for(auto & kv : graph_i.adjacent(i)) {
        std::string u = kv.first;
        double r = kv.second;
        if(!S_t.count(u)) { continue; }
        if(!sum_all.count(i)) {
          sum_all[i] = r;
          sum2_all[i] = r * r;
          nall[i] = 1;
        } else {
          sum_all[i] += r;
          sum2_all[i] += r * r;
          nall[i] += 1;
        }
      }
    }

    // splitting item selection:
    for(auto & i : items) {
      bool flag = false;
      for(size_t indx = 0; indx < result.size(); ++indx) {
        if(result[indx] == i) {
          flag = true;
          break;
        }
      }
      if(flag) { continue; }
      err[i] = 0.;
      std::unordered_map<std::string, double> sum_tL, sum2_tL, sum_tH, sum2_tH;
      std::unordered_map<std::string, int> n_tL, n_tH;

      for(auto & kv : graph_i.adjacent(i)) {
        bool flag_L = false, flag_H = false;
        std::string u = kv.first; 
        double r = kv.second;
        if(!S_t.count(u)) { 
          continue; 
        }
        if(r >= 4) {
          flag_L = true;
        } else {
          flag_H = true;
        }
        for(auto & kkv : graph_u.adjacent(u)) {
          //complexity += 1;
          std::string item_j = kkv.first;
          double r_uj = kkv.second;
          if(flag_L) {
            if(!sum_tL.count(item_j)) {
              sum_tL[item_j] = r_uj;
              sum2_tL[item_j] = r_uj * r_uj;
              n_tL[item_j] = 1;
            } else {
              sum_tL[item_j] += r_uj;
              sum2_tL[item_j] += r_uj * r_uj;
              n_tL[item_j] += 1;
            }
          }
          if(flag_H) {
            if(!sum_tH.count(item_j)) {
              sum_tH[item_j] = r_uj;
              sum2_tH[item_j] = r_uj * r_uj;
              n_tH[item_j] = 1;
            } else {
              sum_tH[item_j] += r_uj;
              sum2_tH[item_j] += r_uj * r_uj;
              n_tH[item_j] += 1;
            }
          }
        } // kkv 
      } // kv 
      // calculate err[i]
      for(auto & ii : items) {
        double e2_tL = foo(sum2_tL[ii], sum_tL[ii], n_tL[ii]);
        double e2_tH = foo(sum2_tH[ii], sum_tH[ii], n_tH[ii]);
        double e2_tU = foo((sum2_all[ii] - sum2_tL[ii] - sum2_tH[ii]), ((sum_all[ii] - sum_tL[ii] - sum_tH[ii])), (nall[ii] - n_tL[ii] - n_tH[ii]));
        err[i] += e2_tL + e2_tH + e2_tU;
      }
      std::cout << i << " 's err is: " << err[i] << std::endl;
      //std::cout << "complexity: " << complexity << std::endl;
    } // i
    
    std::string partition_id;
    double min_err = DBL_MAX;
    for(auto & kv : err) { 
      std::string i = kv.first;
      double e = kv.second;
      if(e < min_err) {
        min_err = e;
        partition_id = i;
      }
    }
    std::cout << partition_id << " with min error: " << min_err << std::endl;
    result.push_back(partition_id);
    
    // creation of t's subtrees
    if(level != height - 1) {
      level += 1;
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

  }

  void solve() {
    auto local_rating_parser1 = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser1 = paracel::gen_parser(local_rating_parser1);
    
    paracel_load_as_graph(graph_u, input, f_parser1);
    auto local_rating_parser2 = [] (const std::string & line) {
      std::vector<std::string> r;
      auto lst = paracel::str_split(line, ',');
      r.push_back(lst[1]);
      r.push_back(lst[0]); 
      r.push_back(lst[2]);
      return r;
    };
    auto f_parser2 = paracel::gen_parser(local_rating_parser2);
    paracel_load_as_graph(graph_i, input, f_parser2);
    sync();
    users = graph_u.vertex_set();
    items = graph_i.vertex_set();

    generate_decision_tree(users);
    for(auto & r : result) {
      std::cout << r << std::endl;
    }
  }

 private:
  std::string input, output;
  int height = 0;
  int level = 0;
  paracel::bigraph<> graph_u;
  paracel::bigraph<> graph_i;
  std::unordered_set<std::string> users;
  std::unordered_set<std::string> items;
  std::vector<std::string> result;
}; // class recommendation_decision_tree

} // namespace paracel

#endif
