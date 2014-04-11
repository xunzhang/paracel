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
                              string _input_rating,
                              string _input_miu,
                              string _input_ubias,
                              string _input_ibias,
                              string _input_ufac,
                              string _input_ifac,
                              string _output, 
                              int _rounds = 1, double _alpha = 0.005, double _beta = 0.01,
                              int limit_s = 3, bool ssp_switch = true) :
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, ssp_switch),
      input_rating(_input_rating),
      input_miu(_input_miu),
      input_ubias(_input_ubias),
      input_ibias(_input_ibias),
      input_ufac(_input_ufac),
      input_ifac(_input_ifac),
      output(_output),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta) {}

  virtual ~content_base_recommendation() {}

  void local_factor_parser(std::unordered_map<string, vector<double> > & var,
                           const vector<string> & linelst, 
                           const char sep1 = '\t',
                           const char sep2 = '|') {
    // init fac_dim
    auto tmp1 = paracel::str_split(linelst[0], sep1);
    auto tmp2 = paracel::str_split(tmp1[1], sep2);
    fac_dim = tmp2.size() + 1;

    for(auto & line : linelst) {
      vector<double> tmp(1.);
      auto v = paracel::str_split(line, sep1);
      auto vv = paracel::str_split(v[1], sep2);
      for(size_t i = 0; i < vv.size(); ++i) {
        tmp.push_back(std::stod(vv[i]));
      }
      var[v[0]] = tmp;
    }
  }

  void local_bias_parser(std::unordered_map<string, double> & var,
                         const vector<string> & linelst, const char sep = '\t') {
    for(auto & line : linelst) {
      auto v = paracel::str_split(line, sep);
      ibias[v[0]] = std::stod(v[1]);
    }
  }

  void init(const string & pattern) {
    // load miu
    auto lines = paracel_loadall(input_miu);
    auto temp = paracel::str_split(lines[0], '\t');
    miu = std::stod(temp[1]);
    
    // load item bias
    lines = paracel_loadall(input_ibias); 
    local_bias_parser(ibias, lines);

    // load item factor
    lines = paracel_load(input_ifac);
    local_factor_parser(item_factor, lines);
    // write item_factor to parameter servers
    for(auto & kv : item_factor) {
      paracel_write(kv.first + "_ifactor", kv.second); // "iid_ifactor"
    }
    
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

    // resize ufactor, ubias here if no ufac specified
    for(auto & kv : usr_rating_lst) {
      ufactor[kv.first] = paracel::random_double_list(fac_dim, 0.1);
      ubias[kv.first] = 0.1 * paracel::random_double();
    }

    /*
    // init ufactor with specified ufac 
    auto select_lambda = [&] (const vector<string> & linelst) {
      vector<double> tmp(0.001);
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, ',');
        for(size_t i = 1; i < v.size(); ++i) {
          tmp.push_back(std::stod(v[i]));
        }
        if(usr_rating_lst.find(v[0]) != usr_rating_lst.end()) {
          ufactor[v[0]] = tmp;
        }
      }
    }; // select_lambda
    // load started user factor
    paracel_sequential_loadall(input_ufac, select_lambda);

    // init ubias with specified ubias
    auto filter_lambda = [&] (const vector<string> & linelst) {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, '\t');
        string uid = v[0];
        double wgt = std::stod(v[1]);
        if(usr_rating_lst.find(uid) != usr_rating_lst.end()) {
          ubias[uid] = wgt;
        }
      }
    };
    // load started user bias
    paracel_sequential_loadall(input_ubias, filter_lambda);
    */
  }

  void learning_1d() {
    init("fmap");
    sync();
    // learning
    for(int rd = 0; rd < rounds; ++rd) {
      // every user 
      for(auto & meta : usr_rating_lst) {
        auto uid = meta.first;
        // traverse rating
        for(auto & kv : meta.second) {
          auto iid = kv.first;
          auto wgt = kv.second;
          if(item_factor.find(iid) == item_factor.end()) {
            auto ifactor = paracel_read<vector<double> >(iid + "_ifactor");
            item_factor[iid] = ifactor;
          }
          double e = wgt - miu - ibias[iid] - ubias[uid] - paracel::dot_product(ufactor[uid], item_factor[iid]);
          double delta = 0.;
          // for k = 0
          delta = alpha * (e * item_factor[iid][0]);
          ufactor[uid][0] += delta;
          // for k != 0
          for(int k = 1; k < fac_dim; ++k) {
            delta = 0.;
            double reg_delta = beta * ufactor[uid][k];
            delta = alpha * (e * item_factor[iid][k] - reg_delta); 
            ufactor[uid][k] += delta;
          }
          // ubias
          ubias[uid] += alpha * (e - beta * ubias[uid]);
        }
      }
    }
  }

  void dump_result() {
    paracel_dump_dict(ufactor, "W_");
    paracel_dump_dict(ubias, "ubias_");
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
  string input_miu;
  string input_ubias;
  string input_ibias;
  string input_ufac;
  string input_ifac;
  string output;
  int fac_dim;
  int rounds;
  double alpha, beta;

  paracel::bigraph<string> rating_graph;  
  std::unordered_map<string, vector<double> > item_factor;
  std::unordered_map<string, vector<std::pair<string, double> > > usr_rating_lst;
  double miu;
  std::unordered_map<string, double> ibias;
  std::unordered_map<string, double> ubias;
  std::unordered_map<string, vector<double> > ufactor;
};

} // namespace paracel

#endif
