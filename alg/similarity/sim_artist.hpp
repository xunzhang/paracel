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
 * Authors: Jason Zhao<jasonzhao@douban.com>
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
			bool _with_confidence = 1):
    paracel::paralg(hosts_dct_str, comm, _output),
	input(_input),
	output(_output),
	simbar(_simbar),
	ktop(_ktop),
	factor_dim(_factor_dim),
	factor_num(_factor_num),
	with_confidence(_with_confidence){}

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

  double cal_sim(const std::vector<double> &a, const std::vector<double> &b) {
	double result = std::inner_product(a.begin(), a.begin() + factor_dim, b.begin(), 0.);
	if (with_confidence)
	  result *= std::sqrt(a[factor_num * factor_dim] * b[factor_num * factor_dim]);
	return result;
  }
  
  void local_learning(const std::unordered_map<std::string, std::vector<double> > & var) {
	for(auto & iv : item_vects) {
	  for(auto & jv : var) {
	    if(iv.first != jv.first) {
		  double sim = cal_sim(iv.second, jv.second);
		  if (sim >= simbar) {
			result[iv.first].push_back(std::make_pair(jv.first, sim));
		  }
		}
	  } // for jv
	} // for iv
  }

  void select_top() {
	auto comp = [] (std::pair<std::string, double> a,
				std::pair<std::string, double> b) {
	  return std::get<1>(a) > std::get<1>(b);
	};
	for(auto & iv : item_vects) {
	  if (result.find(iv.first) == result.end())
		continue;
	  std::sort(result[iv.first].begin(), result[iv.first].end(), comp);
	  if (result[iv.first].size() > (size_t)ktop)
		result[iv.first].resize(ktop);
	}
  }
  
public:
  virtual void solve() {
	auto lines = paracel_load(input);
	local_parser(item_vects, lines);
	std::cout << "local parser done" << std::endl;
    auto handler = [&](const std::vector<std::string> & linelst) {
	  std::unordered_map<std::string, std::vector<double> > other_item_vects;
	  local_parser(other_item_vects, linelst);
	  local_learning(other_item_vects);
    };
	paracel_loadall_handle(input, handler);
	select_top();
	std::cout << "learning done" << std::endl;
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
  std::unordered_map<std::string, std::vector<double> > item_vects;
  std::unordered_map<std::string, std::vector<std::pair<std::string, double> > >result;
};

} // namespace paracel

#endif
