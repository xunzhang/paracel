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

class content_base_recommendation: public paracel::paralg {

 public:
  content_base_recommendation(paracel::Comm comm, 
                              string hosts_dct_str,
                              string _input_rating,
                              string _input_miu,
                              string _input_ubias,
                              string _input_ibias,
                              string _input_ufac,
                              string _input_ifac,
                              string _output, 
                              int _rounds = 1, 
                              double _alpha = 0.001, 
                              double _beta = 0.01,
                              int limit_s = 0, 
                              bool ssp_switch = true) :
      paracel::paralg(hosts_dct_str, 
                      comm, 
                      _output, 
                      _rounds, 
                      limit_s, 
                      ssp_switch),
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

  void init(const string & pattern) {
    // load miu
    auto lines = paracel_loadall(input_miu);
    auto temp = paracel::str_split(lines[1], '\t');
    miu = std::stod(temp[1]);
    
    // load item bias
    lines = paracel_loadall(input_ibias); 
    auto local_ibias_parser = [&] (const vector<string> & linelst,
                                   const char sep = '\t') {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, sep);
        ibias[v[0]] = std::stod(v[1]);
      }
    };
    local_ibias_parser(lines, '\t');
    lines.resize(0);
    std::cout << "print: " << ibias.size() << std::endl;

/*
    // load some of ifactor
    lines = paracel_load(input_ifac);
    auto local_ifac_parser = [&] (const vector<string> & linelst,
                                  const char sep1 = '\t',
                                  const char sep2 = '|') {
      auto tmp1 = paracel::str_split(linelst[0], sep1);
      auto tmp2 = paracel::str_split(tmp1[1], sep2);
      // init fac_dim
      fac_dim = tmp2.size();
      
      for(auto & line : linelst) {
        vector<double> tmp;
        auto v = paracel::str_split(line, sep1);
        auto vv = paracel::str_split(v[1], sep2);
        for(size_t i = 0; i < vv.size(); ++i) {
          tmp.push_back(std::stod(vv[i]));
        }
        ifactor[v[0]] = tmp;
      }
    };
    local_ifac_parser(lines, '\t', '|');
    lines.resize(0);
*/

    // init global ifactor
    if(get_worker_id() == 0) {
      auto handler_lambda = [&] (const vector<string> & linelst) {
        for(auto & line : linelst) {
          vector<double> tmp;
          auto v = paracel::str_split(line, '\t');
          auto vv = paracel::str_split(v[1], '|');
          for(size_t i = 0;i < vv.size(); ++i) {
            tmp.push_back(std::stod(vv[i]));
          }
          ifactor[v[0]] = tmp;
          paracel_write(v[0] + "_ifactor", tmp); // key: "iid_ifactor"
        }
      };
      paracel_sequential_loadall(input_ifac, handler_lambda);
    }
    sync();
    ifactor.clear();
    
    // load bigraph
    auto local_rating_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto rating_parser = paracel::gen_parser(local_rating_parser);
    paracel_load_as_graph(rating_graph, 
                          input_rating, 
                          rating_parser, 
                          pattern);
    // split bigraph into user rating list
    auto split_lambda = [&] (const std::string & a,
                             const std::string & b,
                             double c) {
      // default fmap: first dim is uid
      usr_rating_lst[a].push_back(
          std::make_pair(b, c)
          );
    };
    rating_graph.traverse(split_lambda);
    std::cout << "traverse done" << std::endl;
    
    // init ufactor with specified ufac 
    auto select_lambda = [&] (const vector<string> & linelst) {
      auto tmp1 = paracel::str_split(linelst[0], '\t');
      auto tmp2 = paracel::str_split(tmp1[1], '|');
      // init fac_dim
      fac_dim = tmp2.size();
      for(auto & line : linelst) {
        vector<double> tmp;
        auto v = paracel::str_split(line, '\t');
        if(usr_rating_lst.count(v[0]) == 0) { continue; }
        auto vv = paracel::str_split(v[1], '|');
        for(size_t i = 0; i < vv.size(); ++i) {
          tmp.push_back(std::stod(vv[i]));
        }
        ufactor[v[0]] = tmp;
      }
    }; // select_lambda
    // load started user factor
    paracel_sequential_loadall(input_ufac, select_lambda);
    std::cout << "load ufactor done" << ufactor.size() << "|" << std::endl;

    // init ubias with specified ubias
    auto filter_lambda = [&] (const vector<string> & linelst) {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, '\t');
        string uid = v[0];
        if(usr_rating_lst.count(uid) == 0) { continue; }
        ubias[uid] = std::stod(v[1]);
      }
    };
    // load started user bias
    paracel_sequential_loadall(input_ubias, filter_lambda);
    std::cout << "load ubias done" << ubias.size() << std::endl;
    
    // resize ufactor/ubias here, with no ufac specified
    for(auto & kv : usr_rating_lst) {
      if(ufactor.count(kv.first) == 0) {
        ufactor[kv.first] = paracel::random_double_list(fac_dim, 0.001);
      }
      if(ubias.count(kv.first) == 0) {
        ubias[kv.first] = 0.001 * paracel::random_double();
      }
    }

  }

  void learning_1d() {
    init("fmap");
    sync();
    std::cout << "init done" << std::endl;
    // learning
    for(int rd = 0; rd < rounds; ++rd) {
      // every user 
      for(auto & meta : usr_rating_lst) {
        auto uid = meta.first;
        // traverse rating
        for(auto & kv : meta.second) {
          auto iid = kv.first;
          auto wgt = kv.second;
	  //assert(ufactor[uid].size() == (size_t)fac_dim);
	  //assert(ifactor[iid].size() == (size_t)fac_dim);
	  //assert(ubias.count(uid) == 1);
	  //assert(ibias.count(iid) == 1);
          if(ifactor.count(iid) == 0) {
            ifactor[iid] = paracel_read<vector<double> >(iid + "_ifactor");
          } 
          double e = wgt - miu - 
              ibias[iid] - ubias[uid] - 
              paracel::dot_product(ufactor[uid], ifactor[iid]);
          
          for(int k = 0; k < fac_dim; ++k) {
            double reg_delta = beta * ufactor[uid][k];
            ufactor[uid][k] += alpha * (e * ifactor[iid][k] - reg_delta);
          }
          // ubias
          ubias[uid] += alpha * (e - beta * ubias[uid]);
          if(ifactor.size() > 10000) {
            ifactor.clear();
          }
        }
      }
    }
    usr_rating_lst.clear();
    ifactor.clear();
    ibias.clear();
  }

  void learning_2d() {
    init("fsmap");
    // TODO
  }
  
  void dump_result() {
    paracel_dump_dict(ufactor, "W_");
    paracel_dump_dict(ubias, "ubias_");
  }

  virtual void solve() {
    learning_1d();
    // learning_2d();
  }

 private:
  string input_rating;
  string input_miu, input_ubias, input_ibias;
  string input_ufac, input_ifac;
  string output;
  int fac_dim, rounds;
  double alpha, beta;
  double miu;

  paracel::bigraph<string> rating_graph;  
  std::unordered_map<string, vector<std::pair<string, double> > > usr_rating_lst;
  std::unordered_map<string, vector<double> > ifactor, ufactor;
  std::unordered_map<string, double> ibias, ubias;
};

} // namespace paracel

#endif
