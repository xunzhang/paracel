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
 * For fset fmt usage
 */

#ifndef FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP
#define FILE_1d4484c9_7a95_33cc_3e27_12fd0376083b_HPP

#include <assert.h>
#include <cmath>
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
			int k = 20) :
    paracel::paralg(hosts_dct_str, comm, _output),
	input(_input),
	output(_output),
	ktop(k) {}

  virtual ~similarity() {}
  
  void local_parser(const std::vector<std::string> & linelst, 
  				const char sep = ',') {
    for(auto & line : linelst) {
      std::vector<double> tmp;
	  auto v = paracel::str_split(line, sep);
	  for(size_t i = 1; i < v.size(); ++i) {
	    tmp.push_back(std::stod(v[i]));
	  }
	  item_vects[v[0]] = tmp;
    }
  }

  void normalize() {
    for(auto & kv : item_vects) {
	  double square_sum = 0.;
	  // calc square sum
	  for(size_t i = 0; i < kv.second.size(); ++i) {
	    square_sum += std::pow(kv.second[i], 2);
	  }
	  // divide
	  for(auto it = kv.second.begin(); it != kv.second.end(); ++it) {
	    *it = *it / std::sqrt(square_sum);
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
  
  unsigned long cvt2num(const std::string & s) {
    return std::stoul(s);
  }

  void learning() {
    paracel_register_bupdate("/mfs/user/wuhong/paracel/alg/similarity/update.so",
						"sim_updater");
    // calc local items firstly
    for(auto & iv : item_vects) {
	  std::vector<std::pair<std::string, double> > tmp1, tmp2;
	  for(auto & jv : item_vects) {
	    if( (iv.first != jv.first) || 
			(cvt2num(iv.first) < cvt2num(jv.first)) ) {
		  double sim = paracel::dot_product(iv.second, jv.second);
		  tmp1.push_back(std::make_pair(jv.first, sim));
		  tmp2.push_back(std::make_pair(iv.first, sim));
	    } 
	    auto k2 = jv.first + "_similarity";
	    if(paracel_contains(k2)) {
	      paracel_bupdate(k2, tmp2); 
	    } else {
	      paracel_write(k2, tmp2);
	    }
	  } // for jv
	  auto k1 = iv.first + "_similarity";
	  if(paracel_contains(k1)) { // sth like setdefault in py
	    paracel_bupdate(k1, tmp1);
	  } else {
	    paracel_write(k1, tmp1);
	  }
    } // for iv
    sync();	

	// calc similarity with items in other proces 
	for(int node_id = 0; node_id < (int)get_worker_size(); ++node_id) {
	  auto id_bag = paracel_read<std::vector<std::string> >(
					"item_bag_" + 
					std::to_string(get_worker_id())
					);
      for(auto & iv : item_vects) {
	    for(auto & iid : id_bag) {
		  if(cvt2num(iv.first) < cvt2num(iid)) {
		    std::vector<std::pair<std::string, double> > tmp1, tmp2;
		    auto jv = paracel_read<std::vector<double> >(iid);
		    double sim = paracel::dot_product(iv.second, jv);
			tmp1.push_back(std::make_pair(iv.first, sim));
			tmp2.push_back(std::make_pair(iid, sim));

			auto k1 = iv.first + "_similarity";
			if(paracel_contains(k1)) {
			  paracel_bupdate(k1, tmp1);
			} else {
			  paracel_write(k1, tmp2);
			}

			auto k2 = iid + "_similarity";
			if(paracel_contains(k2)) {
			  paracel_bupdate(k2, tmp2);
			} else {
			  paracel_write(k2, tmp2);
			}
		  }
		} // id_bag
	  } // for iv
	} // bcast_ring 
	sync();
    
	auto comp = [] (std::pair<std::string, double> a,
				std::pair<std::string, double> b) {
	  return std::get<1>(a) > std::get<1>(b);
	};
	// get ktop
	for(auto & iv : item_vects) {
	  auto key = iv.first + "_similarity";
	  auto lst = paracel_read<std::vector<std::pair<std::string, double> > >(key);
	  for(auto & debug : lst) {
	    std::cout << debug.first << "kuang" << std::endl;
	  }
	  std::sort(lst.begin(), lst.end(), comp);
	  assert((size_t)ktop < lst.size());
	  lst.resize(ktop);
	  result[iv.first] = lst;
	}
	sync();
  }

  void dump_result() {
    paracel_dump_dict(result);
  }

  virtual void solve() {
    auto lines = paracel_load(input);
	local_parser(lines);
	normalize(); // normalize here to reduce calculation
	init_paras();
	sync();
	learning();
  }

private:
  std::string input;
  std::string output;
  int ktop = 20;
  std::unordered_map<std::string, std::vector<double> > item_vects;
  std::vector<std::string> item_bag;
  std::unordered_map<std::string, std::vector<std::pair<std::string, double> > >result;
};

}

#endif
