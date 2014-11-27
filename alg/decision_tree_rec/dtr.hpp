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
#include <queue>
#include <eigen3/Eigen/Sparse>
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

namespace paracel {

class recommendation_decision_tree : public paralg {

 public:
  recommendation_decision_tree(paracel::Comm comm, std::string hosts_dct_str,
                               std::string input1, std::string input2, std::string input3, 
                               std::string output, 
                               int height, bool depth_termination, bool alpha_termination, int _alpha) : 
   paracel::paralg(hosts_dct_str, comm, output, 1, 0, false), 
   input1(input1), 
   input2(input2),
   input3(input3),
   height(height), 
   with_depth(depth_termination), 
   with_alpha(alpha_termination), 
   alpha(_alpha) {
    //bigraph_u = new bigraph_continuous(input1);
    //bigraph_i = new bigraph_continuous(input2); 
    //for(int i = 0; i < bigraph_u->v(); ++i) {
    //  users.insert(i);
    //}
    auto parser_lambda = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(parser_lambda);
    paracel_load_as_graph(local_bigraph_i, input3, f_parser, "fmap", false);
    //paracel_load_as_graph(local_bigraph_i, row_map, col_map, input3, f_parser, "fmap", false);
    std::cout << local_bigraph_i.v() << "|" << row_map.size() << "|" << col_map.size() << std::endl;
    //local_bigraph_i.resize(0);
    row_map.clear();
    col_map.clear();
    while(1) {;}
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
    if(with_depth) {
      if(level == height) {
        std::cout << level << "|" << height << std::endl;
        leaf_user_lst.push_back(S_t);
        return;
      }
    }
    if(with_alpha) {
      auto accum_lambda = [&] (int init, int v) {
        return init + bigraph_u->adjacent(v).size();
      };
      int accum = std::accumulate(S_t.begin(), S_t.end(), 0, accum_lambda);
      if(accum < alpha) {
        std::cout << accum << std::endl;
        leaf_user_lst.push_back(S_t);
        return;
      }
    }
    
    int item_sz = bigraph_i->v();
    std::vector<double> err(item_sz, 0.), sum_all(item_sz, 0.), sum2_all(item_sz, 0.);
    std::vector<int> nall(item_sz, 0);

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
    for(int i = 0; i < (int)local_bigraph_i.v(); ++i) {
      int o_i = row_map[i];
      if(std::find(result.begin(), result.end(), o_i) != result.end()) {
        err[o_i] = DBL_MAX;
        continue;
      }
      std::vector<double> sum_tL(item_sz, 0.), sum2_tL(item_sz, 0.);
      std::vector<double> sum_tH(item_sz, 0.), sum2_tH(item_sz, 0.);
      std::vector<int> n_tL(item_sz, 0), n_tH(item_sz, 0);

      for(auto & pair : bigraph_i->adjacent(o_i)) {
        bool flag_L = false, flag_H = false;
        int u = pair.first; double r = pair.second;
        if(!S_t.count(u)) { continue; }
        if(r >= 4) {
          flag_L = true;
        } else {
          flag_H = true;
        }
        for(auto & pair_inside: bigraph_u->adjacent(u)) {
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
      // calculate err[o_i]
      for(int ii = 0; ii < item_sz; ++ii) {
        double e2_tL = foo(sum2_tL[ii], sum_tL[ii], n_tL[ii]);
        double e2_tH = foo(sum2_tH[ii], sum_tH[ii], n_tH[ii]);
        double e2_tU = foo((sum2_all[ii] - sum2_tL[ii] - sum2_tH[ii]), 
                           ((sum_all[ii] - sum_tL[ii] - sum_tH[ii])), 
                           (nall[ii] - n_tL[ii] - n_tH[ii]));
        err[o_i] += e2_tL + e2_tH + e2_tU;
      }
    } // i
    
    std::cout << err.size() << std::endl;

    for(int i = 0; i < item_sz; ++i) {
      get_comm().allreduce(err[i]);
    }
    
    auto result_it = std::minmax_element(err.begin(), err.end());
    int partition_id = result_it.first - err.begin();

    std::cout << "debug: " << level << std::endl; 
    std::cout << partition_id << " with min error: " 
        << err[partition_id] << " and max error: " 
        << err[result_it.second - err.begin()] << std::endl;

    result.push_back(partition_id);

    auto contain_lambda = [&] (std::pair<int, double> pr) {
      return pr.first == partition_id;
    };

    // creation of t's subtrees
    std::unordered_set<int> L, H, U;
    for(auto & u : S_t) {
      auto it = std::find_if(bigraph_u->adjacent(u).begin(), bigraph_u->adjacent(u).end(), contain_lambda);
      if(it == bigraph_u->adjacent(u).end()) {
        U.insert(u);
      } else {
        if((*it).second >= 4.) {
          L.insert(u);
        } else {
          H.insert(u);
        }
      }
    }
    q.push(L);
    q.push(H);
    q.push(U);
  }

  void solve() {
    int cnt = 0;
    int last_cnt = 0;
    q.push(users);
    while(!q.empty()) {
      auto st = q.front();
      q.pop();
      generate_decision_tree(st);
      if((cnt - last_cnt) == 0 || (cnt - last_cnt) == pow(3, level)) {
        last_cnt = cnt;
        level += 1;
      }
      cnt += 1;
    } // while
    for(auto & r : result) {
      std::cout << "result: " << r << std::endl;
    }
    //gen_recommendation();
  }

 private:
  std::string input1, input2, input3, output;
  int height = 0;
  bool with_depth;
  bool with_alpha;
  int alpha = 0;
  int level = 0;
  paracel::bigraph_continuous *bigraph_u;
  paracel::bigraph_continuous *bigraph_i;
  paracel::bigraph<int> local_bigraph_i;
  //paracel::bigraph_continuous local_bigraph_i;
  std::unordered_map<int, int> row_map, col_map;
  std::unordered_set<int> users;
  std::queue<std::unordered_set<int> > q;
  std::vector<int> result;
  std::vector<std::unordered_set<int> > leaf_user_lst;
  std::vector<std::vector<int> > rec_item_lst;
}; // class recommendation_decision_tree

} // namespace paracel

#endif
