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

#ifndef FILE_ce4b7e1a_e522_9f78_ee51_c53f717fb82e_HPP
#define FILE_ce4b7e1a_e522_9f78_ee51_c53f717fb82e_HPP

#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

using std::string;
using std::vector;

namespace paracel {

auto local_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
};

class matrix_factorization: public paracel::paralg {

 public:
  matrix_factorization(paracel::Comm comm, string hosts_dct_str,
                       string _input, string _output, string method = "normal",
                       int k = 80, int _rounds = 1, 
                       double _alpha = 0.005, double _beta = 0.01, bool _debug = false,
                       int limit_s = 3, bool ssp_switch = true) : 
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, ssp_switch),
      input(_input),
      output(_output), 
      learning_method(method),
      fac_dim(k),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta),
      debug(_debug) {}

  virtual ~matrix_factorization() {}

  inline double estimate(const std::string & uid, const std::string & iid) {
    double predict = 0.;
    double tmp = miu + usr_bias[uid] + item_bias[iid] + paracel::dot_product(W[uid], H[iid]);
    if(tmp > 5.) {
      predict = 5.;
    } else if(tmp < 1.) {
      predict = 1.;
    } else {
      predict = tmp;
    }
    return predict;
  }

  double cal_rmse() {
    auto worker_comm = get_comm();
    rmse = 0.;
    auto rmse_lambda = [&] (const std::string & uid,
                            const std::string & iid,
                            double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    rating_graph.traverse(rmse_lambda);
    double rmse_sum = rmse;
    int sz_sum = rating_sz;
    sync();
    worker_comm.allreduce(rmse_sum);
    worker_comm.allreduce(sz_sum);
    return sqrt(rmse_sum / sz_sum);
  }

  void init_parameters() {
    auto worker_comm = get_comm(); 
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(rating_graph, input, f_parser, "fsmap");
    //if(debug) { aracel_load_as_graph(test_rating_graph, test_input, f_parser, "fsmap"); }
    std::cout << "load done" << std::endl;
    rating_sz = rating_graph.e();
    auto init_lambda = [&] (const std::string & a,
                            const std::string & b,
                            double c) {
      usr_bag[a] = '7';
      item_bag[b] = '7';
      miu += c;
    };
    rating_graph.traverse(init_lambda);
    std::cout << "traverse done" << std::endl;
    sync();
    worker_comm.allreduce(miu);
    long rating_sz_tmp = rating_sz;
    worker_comm.allreduce(rating_sz_tmp);
    miu /= rating_sz_tmp;
    
    for(auto & kv : usr_bag) {
      W[kv.first] = paracel::random_double_list(fac_dim, 0.1);
      usr_bias[kv.first] = 0.1 * paracel::random_double();
    }
    for(auto & kv : item_bag) {
      H[kv.first] = paracel::random_double_list(fac_dim, 0.1);
      item_bias[kv.first] = 0.1 * paracel::random_double();
    }

    // init push
    id = get_worker_id();
    paracel_register_bupdate("/mfs/user/wuhong/paracel/local/lib/libmf_update.so",
                             "cnt_updater");
    std::cout << "usr_bag size: " << usr_bag.size() << std::endl;
    std::cout << "item bag size: " << item_bag.size() << std::endl;
    paracel::dict_type<paracel::str_type, paracel::list_type<double> > local_W_dct, local_H_dct;
    paracel::dict_type<paracel::str_type, double> local_usr_bias_dct, local_item_bias_dct;
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W_" + uid;
      std::string ub_key = "usr_bias_" + uid;
      paracel_write(W_key, W[uid]);
      paracel_write(ub_key, usr_bias[uid]);
      paracel_bupdate(uid + "_u_cnt", 1);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H_" + iid;
      std::string ib_key = "item_bias_" + iid;
      paracel_write(H_key, H[iid]);
      paracel_write(ib_key, item_bias[iid]);
      paracel_bupdate(iid + "_i_cnt", 1);
    }
    std::cout << "init push done" << std::endl;
    sync();
    for(auto & kv : usr_bag) {
      wgtx_map[kv.first] = 1. / static_cast<double>(paracel_read<int>(kv.first + "_u_cnt"));
    }
    for(auto & kv : item_bag) {
      wgty_map[kv.first] = 1. / static_cast<double>(paracel_read<int>(kv.first + "_i_cnt"));
    }
    sync();
    std::cout << "init done" << std::endl;
  }

  void read_mf_paras() {
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W_" + uid;
      std::string ub_key = "usr_bias_" + uid;
      W[uid] = paracel_read<vector<double> >(W_key);
      usr_bias[uid] = paracel_read<double>(ub_key);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H_" + iid;
      std::string ib_key = "item_bias_" + iid;
      H[iid] = paracel_read<vector<double> >(H_key);
      item_bias[iid] = paracel_read<double>(ib_key);
    }
  }

  void update_mf_fac(std::unordered_map<string, vector<double> > & old_W,
                     std::unordered_map<string, vector<double> > & old_H) {
    paracel::str_type file_name = "/mfs/user/wuhong/paracel/build/lib/libmf_update.so";
    paracel::str_type func_name = "mf_fac_updater";
    vector<double> fac_delta(fac_dim);
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W_" + uid;
      for(int i = 0; i < fac_dim; ++i) {
        fac_delta[i] = wgtx_map[uid] * (W[uid][i] - old_W[uid][i]);
      }
      paracel_bupdate(W_key, fac_delta, file_name, func_name);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H_" + iid;
      for(int i = 0; i < fac_dim; ++i) {
        fac_delta[i] = wgty_map[iid] * (H[iid][i] - old_H[iid][i]);
      }
      paracel_bupdate(H_key, fac_delta, file_name, func_name);
    }
  }

  void update_mf_bias(std::unordered_map<string, double> & old_ubias, 
                      std::unordered_map<string, double> & old_ibias) {
    paracel::str_type file_name = "/mfs/user/wuhong/paracel/build/lib/libmf_update.so";
    paracel::str_type func_name = "mf_bias_updater";
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string ub_key = "usr_bias_" + uid;
      paracel_bupdate(ub_key, wgtx_map[uid] * (usr_bias[uid] - old_ubias[uid]), file_name, func_name);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string ib_key = "item_bias_" + iid;
      paracel_bupdate(ib_key, wgty_map[iid] * (item_bias[iid] - old_ibias[iid]), file_name, func_name);
    }
  }

  void learning() {
    std::vector<double> delta_W(fac_dim), delta_H(fac_dim);
    auto kernel_lambda = [&] (const std::string & uid,
                              const std::string & iid,
                              double rating) {
      double e = rating - estimate(uid, iid);
      for(int i = 0; i < fac_dim; ++i) {
        delta_W[i] = alpha * (2 * e * H[iid][i] - beta * W[uid][i]);
        delta_H[i] = alpha * (2 * e * W[uid][i] - beta * H[iid][i]);
      }
      for(int i = 0; i < fac_dim; ++i) {
        W[uid][i] += delta_W[i];
        H[iid][i] += delta_H[i];
      }
      usr_bias[uid] += alpha * (2 * e - beta * usr_bias[uid]);
      item_bias[iid] += alpha * (2 * e - beta * item_bias[iid]);
    };
    std::unordered_map<string, vector<double> > old_W, old_H;
    std::unordered_map<string, double> old_ubias, old_ibias;

    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      std::cout << "read" << std::endl;
      // read paras from servers
      read_mf_paras();
      std::cout << "read done" << std::endl;
      // record locally
      for(auto & kv : usr_bag) {
        auto uid = kv.first;
        if(old_W[uid].size() != (size_t)fac_dim) {
          old_W[uid].resize(fac_dim);
        }
        for(int i = 0; i < fac_dim; ++i) {
          old_W[uid][i] = W[uid][i];
        }
        old_ubias[uid] = usr_bias[uid];
      }
      for(auto & kv : item_bag) {
        auto iid = kv.first;
        if(old_H[iid].size() != (size_t)fac_dim) {
          old_H[iid].resize(fac_dim);
        }
        for(int i = 0; i < fac_dim; ++i) {
          old_H[iid][i] = H[iid][i];
        }
        old_ibias[iid] = item_bias[iid];
      }
      std::cout << "record done" << std::endl;
      // update paras locally
      rating_graph.traverse(kernel_lambda);
      std::cout << "traverse done" << std::endl;
      // update paras to servers
      update_mf_fac(old_W, old_H);
      sync(); // notice 
      update_mf_bias(old_ubias, old_ibias);
      sync();
      std::cout << "update done" << std::endl;
      iter_commit();
    }
    // last read
    read_mf_paras();
    usr_bag.clear();
    item_bag.clear();
  }

  virtual void solve() {
    init_parameters(); 
    sync();
    if(learning_method == "ipm") {
      set_total_iters(rounds);
      learning();
    } else {}
    sync();
  }

  void dump_result() {
    auto worker_comm = get_comm(); 
    long rating_sz_tmp = rating_sz;
    worker_comm.allreduce(rating_sz_tmp);

    if(id == 0) {
      std::unordered_map<string, double> dump_miu;
      dump_miu["miu"] = miu;
      dump_miu["rating_sz"] = static_cast<double>(rating_sz_tmp);
      paracel_dump_dict(dump_miu, "miu_");
      
      auto tmp_usr_bias = paracel_read_special<double>(
          "/mfs/user/wuhong/paracel/local/lib/libmf_filter.so", 
          "mf_ubias_filter"
          );
      auto tmp_item_bias = paracel_read_special<double>(
          "/mfs/user/wuhong/paracel/local/lib/libmf_filter.so", 
          "mf_ibias_filter"
          );
      auto tmp_W = paracel_read_special<vector<double> >(
          "/mfs/user/wuhong/paracel/local/lib/libmf_filter.so", 
          "mf_W_filter"
          );
      auto tmp_H = paracel_read_special<vector<double> >(
          "/mfs/user/wuhong/paracel/local/lib/libmf_filter.so", 
          "mf_H_filter"
          );
      
      std::unordered_map<string, double> dump_usr_bias, dump_item_bias;
      std::unordered_map<string, vector<double> > dump_W, dump_H;
      
      auto tear_lambda = [] (const string & str) {
        auto pos1 = str.rfind('_') + 1;
        string s = str.substr(pos1, str.size());
        return s;
      };

      for(auto & kv : tmp_usr_bias) {
        string temp = tear_lambda(kv.first);
        dump_usr_bias[temp] = kv.second;
      }
      for(auto & kv : tmp_item_bias) {
        string temp = tear_lambda(kv.first);
        dump_item_bias[temp] = kv.second;
      }
      for(auto & kv : tmp_W) {
        string temp = tear_lambda(kv.first);
        dump_W[temp] = kv.second;
      }
      for(auto & kv : tmp_H) {
        string temp = tear_lambda(kv.first);
        dump_H[temp] = kv.second;
      }
      
      auto dump_lambda = [&] () {
        paracel_dump_dict(dump_W, "W_");
        paracel_dump_dict(dump_usr_bias, "ubias_");
        paracel_dump_dict(dump_H, "H_");
        paracel_dump_dict(dump_item_bias, "ibias_");
      };
      dump_lambda();
      std::cout << "dumpdone" << std::endl;
    }
  }

 private:
  int id;
  string input, output;
  string learning_method;
  int fac_dim; // factor dim
  int rounds;
  double alpha, beta;
  bool debug;
  vector<double> loss_error;

  size_t rating_sz = 0;
  double miu = 0., rmse = 0.;
  //double wgt_x = 0., wgt_y = 0.;
  paracel::bigraph<std::string> rating_graph;
  
  std::unordered_map<string, double> wgtx_map, wgty_map;
  std::unordered_map<string, char> usr_bag, item_bag;
  std::unordered_map<string, vector<double> > W, H;
  std::unordered_map<string, double> usr_bias, item_bias;
};

} // namespace paracel

#endif
