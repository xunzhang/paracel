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

#ifndef FILE_a3d185a2_149d_fe7f_90a8_fbbbc7a8209a_H 
#define FILE_a3d185a2_149d_fe7f_90a8_fbbbc7a8209a_H

#include <vector>
#include <string>
#include <unordered_map>
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

using std::vector;
using std::string;

namespace paracel {

auto local_rating_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
};

class content_base_recommendation: public paracel::paralg {

public:
  content_base_recommendation(paracel::Comm comm, string hosts_dct_str,
                              string _input_rating, string _input_ifac, string _input_ufac, string _output, 
                              int _rounds = 1, double _alpha = 0.005, double _beta = 0.01,
                              int limit_s = 3, bool ssp_switch = true) :
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, ssp_switch),
      input_rating(_input_rating),
      input_ifac(_input_ifac),
      input_ufac(_input_ufac),
      output(_output),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta) {}
  
  virtual ~content_base_recommendation() {}
  
  void local_factor_parser(std::unordered_map<string, vector<double> > & var,
                           const vector<string> & linelst, const char sep = ',') {
    // init fac_dim
    auto tmp = paracel::str_split(linelst[0], sep);
    fac_dim = tmp.size(); // tmp.size() -1 + 1

    for(auto & line : linelst) {
      vector<double> tmp(1.);
      auto v = paracel::str_split(line, sep);
      for(size_t i = 1; i < v.size(); ++i) {
        tmp.push_back(std::stod(v[i]));
      }
      var[v[0]] = tmp;
    }
  }

  void init(const string & pattern) {
    // load item factor
    auto lines = paracel_load(input_ifac);
    local_factor_parser(item_factor, lines);
    // write item_factor to parameter servers
    for(auto & kv : item_factor) {
      paracel_write(kv.first + "_ifactor", kv.second);
    }
    // resize theta here if no ufac specified
    // TODO

    // load bigraph
    auto rating_parser = paracel::gen_parser(local_rating_parser);
    paracel_load_as_graph(rating_graph, input_rating, rating_parser, pattern);
    // split bigraph into user rating list
    auto split_lambda = [&] (const std::string & a,
                       const std::string & b,
                       double c) {
      // default fmap: first dim is uid
      usr_rating_lst[a].push_back(std::make_pair(b, c));
    };
    rating_graph.traverse(split_lambda);

    // init theta
    auto select_lambda = [&] (const vector<string> & linelst) {
      vector<double> tmp(0.001);
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, ',');
        for(size_t i = 1; i < v.size(); ++i) {
          tmp.push_back(std::stod(v[i]));
        }
        if(usr_rating_lst.find(v[0]) != usr_rating_lst.end()) {
          theta[v[0]] = tmp;
        }
      }
    }; // select_lambda
    // load started user factor
    paracel_sequential_loadall(input_ufac, select_lambda);
  }

  void learning_1d() {
    init("fmap");
    // learning
    for(int rd = 0; rd < rounds; ++rd) {
      // every user 
      for(auto & meta : usr_rating_lst) {
        auto uid = meta.first;
        // traverse factor
        for(int k = 0; k < fac_dim; ++k) {
          double delta = 0.;
          double reg_delta = beta * theta[uid][k];
          if(k == 0) {
            for(auto & kv : meta.second) {
              auto iid = kv.first;
              auto wgt = kv.second;
              if(item_factor.find(iid) == item_factor.end()) {
                auto ifactor = paracel_read<vector<double> >(iid + "ifactor");
                item_factor[iid] = ifactor;
              }
              delta += (wgt - paracel::dot_product(theta[uid], item_factor[iid])) * item_factor[iid][k];
            }
          } else {
            for(auto & kv : meta.second) {
              auto iid = kv.first;
              auto wgt = kv.second;
              if(item_factor.find(iid) == item_factor.end()) {
                auto ifactor = paracel_read<vector<double> >(iid + "ifactor");
                item_factor[iid] = ifactor;
              }
              delta += (wgt - paracel::dot_product(theta[uid], item_factor[iid])) * item_factor[iid][k] + reg_delta;
            }
          }
          delta *= alpha;
          theta[uid][k] += delta;
        }
      }
    }
  }
  
  void dump_result() {
    paracel_dump_dict(theta);
  }

  void learning_2d() {
    init("fsmap");
    // TODO
  }

  virtual void solve() {
    learning_1d();
    // learning_2d();
  }

private:
  string input_rating;
  string input_ifac;
  string input_ufac;
  string output;
  int fac_dim;
  int rounds;
  double alpha, beta;

  paracel::bigraph<string> rating_graph;  
  std::unordered_map<string, vector<double> > item_factor;
  std::unordered_map<string, vector<std::pair<string, double> > > usr_rating_lst;
  std::unordered_map<string, vector<double> > theta;
};

} // namespace paracel

#endif
