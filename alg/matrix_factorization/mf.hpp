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
    return miu + usr_bias[uid] + item_bias[iid] + paracel::dot_product(W[uid], H[iid]);
  }

  double cal_rmse() {
    rmse = 0.;
    auto rmse_lambda = [&] (const std::string & uid,
    			const std::string & iid,
			double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    rating_graph.traverse(rmse_lambda);
    auto worker_comm = get_comm();
    double rmse_sum = rmse;
    int sz_sum = rating_sz;
    worker_comm.allreduce(rmse_sum);
    worker_comm.allreduce(sz_sum);
    return sqrt(rmse_sum / sz_sum);
  }

  void init_parameters() {
    
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(rating_graph, input, f_parser, "fsmap");
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
    miu /= rating_sz;
    
    for(auto & kv : usr_bag) {
      W[kv.first] = paracel::random_double_list(fac_dim, 0.1);
      usr_bias[kv.first] = 0.1 * paracel::random_double();
    }
    for(auto & kv : item_bag) {
      H[kv.first] = paracel::random_double_list(fac_dim, 0.1);
      item_bias[kv.first] = 0.1 * paracel::random_double();
    }
    
    // init push
    get_decomp_info(npx, npy);
    wgt_x = 1. / npy;
    wgt_y = 1. / npx;
    id = get_worker_id();
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W[" + uid + "]_" + std::to_string(id / npy);
      std::string ub_key = "usr_bias[" + uid + "]_" + std::to_string(id / npy);
      paracel_write(W_key, W[uid]);
      paracel_write(ub_key, usr_bias[uid]);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H[" + iid + "]_" + std::to_string(id % npy);
      std::string ib_key = "item_bias[" + iid + "]_" + std::to_string(id % npy);
      paracel_write(H_key, H[iid]);
      paracel_write(ib_key, item_bias[iid]);
    }
    sync();
    std::cout << "init done" << std::endl;
  }
  
  void read_mf_paras() {
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W[" + uid + "]_" + std::to_string(id / npy);
      std::string ub_key = "usr_bias[" + uid + "]_" + std::to_string(id / npy);
      W[uid] = paracel_read<vector<double> >(W_key);
      usr_bias[uid] = paracel_read<double>(ub_key);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H[" + iid + "]_" + std::to_string(id % npy);
      std::string ib_key = "item_bias[" + iid + "]_" + std::to_string(id % npy);
      H[iid] = paracel_read<vector<double> >(H_key);
      item_bias[iid] = paracel_read<double>(ib_key);
    }
  }

  void update_mf_fac(std::unordered_map<string, vector<double> > & old_W,
  		std::unordered_map<string, vector<double> > & old_H) {
    /*
    paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/libmf_update.so",
    			"mf_fac_updater");
    */
    paracel::str_type file_name = "/mfs/user/wuhong/paracel/build/lib/libmf_update.so";
    paracel::str_type func_name = "mf_fac_updater";
    vector<double> delta_W(fac_dim), delta_H(fac_dim);
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string W_key = "W[" + uid + "]_" + std::to_string(id / npy);
      for(int i = 0; i < fac_dim; ++i) {
        delta_W[i] = wgt_x * (W[uid][i] - old_W[uid][i]);
      }
      //paracel_bupdate(W_key, delta_W);
      paracel_bupdate(W_key, delta_W, file_name, func_name);
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string H_key = "H[" + iid + "]_" + std::to_string(id % npy);
      for(int i = 0; i < fac_dim; ++i) {
        delta_H[i] = wgt_y * (H[iid][i] - old_H[iid][i]);
      }
      //paracel_bupdate(H_key, delta_H);
      paracel_bupdate(H_key, delta_H, file_name, func_name);
    }
  }
  
  void update_mf_bias(std::unordered_map<string, double> & old_ubias, 
  		std::unordered_map<string, double> & old_ibias) {
    /*
    paracel_register_bupdate("/mfs/user/wuhong/paracel/build/lib/libmf_update.so",
    			"mf_bias_updater");
    */
    paracel::str_type file_name = "/mfs/user/wuhong/paracel/build/lib/libmf_update.so";
    paracel::str_type func_name = "mf_bias_updater";
    for(auto & kv : usr_bag) {
      auto uid = kv.first;
      std::string ub_key = "usr_bias[" + uid + "]_" + std::to_string(id / npy);
      paracel_bupdate(ub_key, wgt_x * (usr_bias[uid] - old_ubias[uid]), file_name, func_name);
      //paracel_bupdate(ub_key, wgt_x * (usr_bias[uid] - old_ubias[uid]));
    }
    for(auto & kv : item_bag) {
      auto iid = kv.first;
      std::string ib_key = "item_bias[" + iid + "]_" + std::to_string(id % npy);
      paracel_bupdate(ib_key, wgt_y * (item_bias[iid] - old_ibias[iid]), file_name, func_name);
      //paracel_bupdate(ib_key, wgt_y * (item_bias[iid] - old_ibias[iid]));
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
	//std::copy(W[uid].begin(), W[uid].end(), old_H[uid].begin());
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
	//std::copy(H[iid].begin(), H[iid].end(), old_H[iid].begin());
	old_ibias[iid] = item_bias[iid];
      }
      std::cout << "record done" << std::endl;
      // update paras locally
      rating_graph.traverse(kernel_lambda);
      std::cout << "traverse done" << std::endl;
      // update paras to servers
      update_mf_fac(old_W, old_H);
      //sync(); // notice 
      update_mf_bias(old_ubias, old_ibias);
      //sync();
      std::cout << "update done" << std::endl;
      iter_commit();
    }
    // last read
    read_mf_paras();
  }

  virtual void solve() {
    init_parameters(); 
    sync();
    if(learning_method == "ipm") {
      set_total_iters(rounds);
      learning();
    } else {
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
  
  int npx = 0, npy = 0, rating_sz = 0;
  double miu = 0., rmse = 0.;
  double wgt_x = 0., wgt_y = 0.;
  paracel::bigraph<std::string> rating_graph;

  std::unordered_map<string, char> usr_bag, item_bag;
  std::unordered_map<string, vector<double> > W, H;
  std::unordered_map<string, double> usr_bias, item_bias;
};

} // namespace paracel

#endif
