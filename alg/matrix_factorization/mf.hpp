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

#include <vector>
#include <string>
#include <unordered_map>

#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

using std::string;
using std::vector<double>;

namespace paracel {

auto local_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
}

class matrix_factorization : public paracel::paralg {

public:
  matrix_factorization(paracel::Comm comm, string hosts_dct_str,
  		string _input, string _output, string method = "normal",
		int k = 80, int _rounds = 1, 
		double alpha = 0.005, double beta = 0.01, bool _debug = false,
		int limit_s = 3, bool ssp_switch = true) : 
		paracel::paralg(host_dct_str, comm, _output, _rounds, limit_s, ssp_switch),
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
    return miu + usr_bias[uid] + item_bias[iid] + paracel::dot_product(p[uid], q[iid]);
  }

  void init_parameters() {
    
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(rating_graph, input, f_parser);
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
    auto id = get_worker_id();
    for(int i = 0; i < usr_bag.size(); ++i) {
      std::string key = "W[" + std::to_string(i) + ",:]_" + std::to_string(id / npy);
      paracel_write(key, W[i]);
    }
    for(int i = 0; i < item_bag.size(); ++i) {
      std::string key = "H[:," + std::to_string(i) + "]_" + std::to_string(id % npy);
      paracel_write(key, H[i]);
    }
    sync();

    std::cout << "init done" << std::endl;
  }

  void learning() {
    init_parameters(); 
    std::vector<double> delta_W(fac_dim), delta_H(fac_dim);
    auto kernel = [&] (const std::string & uid,
    			const std::string & iid,
			double rating) {
      double e = rating - estimate(uid, iid);
      for(int i = 0; i < fac_dim; ++i) {
        delta_W[i] = alpha * (2 * e * H[iid][i] - beta * W[uid][i]);
	delta_H[i] = alpha * (2 * e * W[uid][i] - beta * H[iid][i]);
      }
      for(int i = 0; i < fac_dim; ++i) {
        W[uid][i] += delta_W[i];
	H[id][i] += delta_H[i];
      }
      usr_bias[uid] += alpha * (2 * e - beta * usr_bias[uid]);
      item_bias[iid] += alpha * (2 * e - beta * item_bias[iid]);
    };
    
    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      std::cout << "rd:" << rd << " started" << std::endl;
      rating_graph.traverse(kernel);
    }
  }

  virtual void solve() {
    load();
    sync();
    learning();
  }

private:
  int fac_dim; // factor dim
  int worker_id;
  int rounds;
  string input, output;
  double alpha, beta;
  bool debug;
  vector<double> loss_error;
  string learning_method;
  
  int npx = 0, npy = 0, rating_sz = 0;
  double miu = 0., rmse = 0.;
  paralg::bigraph<std::string> rating_graph;

  std::unordered_map<string, char> usr_bag, item_bag;
  std::unordered_map<string, vector<double> > W, H;
  std::unordered_map<string, double> usr_bias, item_bias;
};

} // namespace paracel

#endif
