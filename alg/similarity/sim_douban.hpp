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
 * For fset fmt usage
 */

#ifndef FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP
#define FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP

#include <assert.h>
#include <cmath>
#include <cfloat>
#include <string>
#include <utility>
#include <algorithm>
#include <unordered_map>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {

class similarity : public paracel::paralg {

public:
  
  similarity(paracel::Comm comm, 
  			std::string hosts_dct_str,
			std::string _input,
			std::string _output,
			double _simbar = 0.0,
			int _ktop = 20,
			int _factor_dim = 100,
			int _factor_num = 5,
			bool _with_confidence = 1,
			std::string _method = "default") :
    paracel::paralg(hosts_dct_str, comm, _output),
	input(_input),
	output(_output),
	simbar(_simbar),
	ktop(_ktop),
	factor_dim(_factor_dim),
	factor_num(_factor_num),
	with_confidence(_with_confidence),
	learning_method(_method){}

  virtual ~similarity() {}

private:
  
  void local_parser(std::unordered_map<std::string, std::vector<double> > & var,
  				const std::vector<std::string> & linelst, 
  				const char sep = ',') {
    for(auto & line : linelst) {
      std::vector<double> tmp;
	  auto v = paracel::str_split(line, sep);
	  for(size_t i = 1; i < v.size(); ++i) {
	    tmp.push_back(std::stod(v[i]));
	  }
	  var[v[0]] = tmp;
    }
  }

  void normalize(std::unordered_map<std::string, std::vector<double> > & var) {
    for(auto & kv : var) {
	  for (int i = 0; i < factor_num; ++i) {
		double normalizer = std::sqrt(std::inner_product(kv.second.begin() + i*factor_dim, kv.second.begin() + (i+1)*factor_dim, kv.second.begin() + i*factor_dim, 0.));
		for (int j = i * factor_dim; j < ((i + 1) * factor_dim); ++j){
		  kv.second[j] = kv.second[j] / normalizer;
		  if (with_confidence)
			kv.second[j] *= kv.second[factor_num * factor_dim];
		}
	  }
	}
  }
  
  void init_paras() {
    for(auto & kv : item_vects) {
	  paracel_write(kv.first, kv.second); // push vector
	  item_bag.push_back(kv.first);
	}
	std::string key = "item_bag_" + std::to_string(get_worker_id());
	paracel_write(key, item_bag); // push iids
  }

  double cal_sim(const std::vector<double> &a, const std::vector<double> &b) {
	std::vector<double> tmp;
	tmp.resize(factor_num);
	for (int i = 0; i < factor_num; ++i)
	  tmp[i] = std::inner_product(a.begin() + i * factor_dim, a.begin() + (i + 1) * factor_dim, b.begin() + i * factor_dim, 0.);
	if (factor_num == 1) {
	  return tmp[0];
	}
	double sum = std::accumulate(tmp.begin(), tmp.end(), 0.0);
	double m = sum / factor_num;
	double se = 0.0;
	for (int i=0; i<factor_num; ++i) {
	  se = se + std::pow((tmp[i] - m), 2);
	}
	double std = std::sqrt(se / (factor_num - 1));
	return m - std;
  }
  
  void local_learning(const std::unordered_map<std::string, std::vector<double> > & var, bool dump_flag = true) {
	auto comp = [] (std::pair<std::string, double> a,
	  			std::pair<std::string, double> b) {
	  return std::get<1>(a) > std::get<1>(b);
	};
	for(auto & iv : item_vects) {
	  for(auto & jv : var) {
	    if(iv.first != jv.first) {
		  double sim = cal_sim(iv.second, jv.second);
		  if (sim >= simbar) {
			result[iv.first].push_back(std::make_pair(jv.first, sim));
		  }
		}
	  } // for jv
	  std::sort(result[iv.first].begin(), result[iv.first].end(), comp);
	  result[iv.first].resize(ktop);
	  if(dump_flag) {
	    dump_result();
	    result.clear();
	  }
	} // for iv
  }

  void learning() {
	local_learning(all_item_vects);
	sync();
  }

  void mls_learning() {
	// learn with local item vectors
	local_learning(item_vects, false);
	auto comp = [] (std::pair<std::string, double> a,
	  			std::pair<std::string, double> b) {
	  return std::get<1>(a) > std::get<1>(b);
	};
	// calc similarity with items in other proces 
	for(int node_id = 0; node_id < (int)get_worker_size(); ++node_id) {
	  if(node_id == get_worker_id()) continue;
	  auto id_bag = paracel_read<std::vector<std::string> >(
					"item_bag_" + 
					std::to_string(node_id)
					);
      for(auto & iv : item_vects) {
	    for(auto & iid : id_bag) {
		  auto jv = paracel_read<std::vector<double> >(iid);
		  double sim = cal_sim(iv.second, jv);
		  if (sim >= simbar) {
			result[iv.first].push_back(std::make_pair(iid, sim));
		  }
		} // id_bag
	    std::sort(result[iv.first].begin(), result[iv.first].end(), comp);
	    result[iv.first].resize(ktop);
	  } // for iv
	} // bcast_ring
	dump_result();
	sync();
  }
  
public:
  virtual void solve() {
	auto lines = paracel_load(input);
	local_parser(item_vects, lines);
	std::cout << "parser done" << std::endl;
	if(learning_method == "default") {
	  auto all_lines = paracel_loadall(input);
	  local_parser(all_item_vects, all_lines);
	  std::cout << "loadall done" << std::endl;
	  normalize(item_vects);
	  normalize(all_item_vects);
	  std::cout << "normalize done" << std::endl;
	  sync();
	  learning();
	} else if(learning_method == "limit_storage") {
	  normalize(item_vects); // normalize here to reduce calculation
	  init_paras();
	  sync();
	  mls_learning();
	} else {}
  }
  
  void dump_result() {
    paracel_dump_dict(result);
  }


private:
  std::string input;
  std::string output;
  double simbar = 0.;
  int ktop = 20;
  int factor_dim = 100;
  int factor_num = 5;
  bool with_confidence = true;
  std::string learning_method;
  std::unordered_map<std::string, std::vector<double> > item_vects;
  std::unordered_map<std::string, std::vector<double> > all_item_vects; // for default matmul usage
  std::vector<std::string> item_bag;
  std::unordered_map<std::string, std::vector<std::pair<std::string, double> > >result;
};

} // namespace paracel

#endif
