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

#ifndef FILE_49843a5e_b2f8_f70c_58c7_1b13de85a6eb_HPP 
#define FILE_49843a5e_b2f8_f70c_58c7_1b13de85a6eb_HPP 

#include <unordered_map>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include "ps.hpp"

namespace paracel {

class hi : public paracel::paralg {

public:
  hi(paracel::Comm comm, std::string hosts_dct_str,
  	std::string _input, std::string _output, std::string para) :
	paracel::paralg(hosts_dct_str, comm, _output, 1, 0, true), 
	input(_input),
	hi_para(para) {
    wid = (size_t)get_worker_id();
  }

  virtual ~hi() {}

  virtual void solve() {
    std::cout << "a" << input << std::endl;
    auto lambda = [&] (paracel::list_type<paracel::str_type> & somelines) {
      for(auto & line : somelines) {
	    if(get_worker_id() == 0) {
	      std::cout << line << std::endl;
		}
	  }
	  if(get_worker_id() == 0) {
	    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
	  }
	};
    //paracel_loadall_handle(input, lambda);
    paracel_loadall(input);
    std::unordered_map<std::string, double> map;
    std::string tt = "a";
    map[tt] = 1.;
    std::cout << "b" << std::endl;
    paracel_dump_dict(map);
    std::cout << "a" << std::endl;
    map.clear();
    tt = "b";
    map[tt] = 2.;
    paracel_dump_dict(map);
    map.clear();
  }

  void init_partition(const std::string & fn,
                      std::unordered_map<
                      std::string, 
                      char> & target) {
    auto handler = [&] (const std::vector<std::string> & linelst,
                        std::unordered_map<
                        std::string,
                        char> & out,
                        const char sep = ',') {
      paracel::scheduler scheduler(get_comm());
      for(auto & line : linelst) {
        auto lst = paracel::str_split(line, sep);
        auto uid = lst[0];
        auto iid = lst[1];
        double rating = std::stod(lst[2]);
        size_t hid = scheduler.select(uid, iid);
        if(hid == wid) {
          out[uid] = 't';
        }
      } // for
    };
    auto handler_wrapper = [&] (const std::vector<std::string> & linelst) {
      handler(linelst, target);
    };
    paracel_loadall_handle(fn, handler_wrapper);
  }

  virtual void opt() {
    //init_partition("/mfs/user/wuhong/plato/data/input/netflix.train", rlst);
    paracel_write("sad", 1.);
    paracel_write("fat", 3.);
    auto dd = paracel_readall<double>();
    for(auto & l : dd) {
      std::cout << l.first << "|" << l.second << std::endl;
    }
  }

  void test() {
    auto f_parser = paracel::gen_parser(paracel::parser_b, '\t', '|');
    //std::string fn = "/mfs/user/wuhong/paracel/data/spectral_clustering/demo.txt";
    std::string fn = "/mfs/user/wuhong/paracel/data/spectral_clustering/demo.txt";
    paracel_load_as_matrix(blk_W, fn, f_parser, "smap", true);
    if(get_worker_id() == 0) {
      std::cout << blk_W << std::endl;
    }
  }

private:
  std::string input;
  std::string hi_para;
  size_t wid;
  std::unordered_map<std::string, char> rlst;
  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_W;
};

}

#endif
