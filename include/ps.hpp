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
#ifndef FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP
#define FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP

#include <dlfcn.h>
#include <fstream>
#include <algorithm>
#include <functional>

#include <boost/filesystem.hpp>
#include <boost/any.hpp>

#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>

#include "paracel_types.hpp"
#include "client.hpp"
#include "utils.hpp"
#include "graph.hpp"
#include "load.hpp"
#include "ring.hpp"
#include "packer.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

class paralg {

private:
  void init_output(const paracel::str_type & folder) {
    // create output folder
    if(worker_comm.get_rank() == 0) {
      boost::filesystem::path path(folder);
      if(boost::filesystem::exists(path)) {
        boost::filesystem::remove_all(path);
      }
      boost::filesystem::create_directory(path);
    }
  }

  // TODO: abstract update_f to para
  void load_update_f(const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if(!handler) {
      std::cerr << "Cannot open library: " << dlerror() << '\n';
      return;
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol: " << dlerror() << '\n';
      dlclose(handler);
      return;
    }
    update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  }

public:
  // interface for direct usage
  paralg(paracel::Comm comm,
  	paracel::str_type op_folder,
  	int o_rounds) : 
		worker_comm(comm), 
		output(op_folder), 
		rounds(o_rounds) {
    init_output(op_folder);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
  }
  
  paralg(paracel::str_type hosts_dct_str, 
  	paracel::Comm comm,
	paracel::str_type op_folder,
	int o_rounds,
	int o_limit_s = 0,
	bool _ssp_switch = false) :
	worker_comm(comm),
	output(op_folder),
	nworker(comm.get_size()),
	rounds(o_rounds),
	limit_s(o_limit_s),
	ssp_switch(_ssp_switch) {
    ps_obj = new parasrv(hosts_dct_str);
    init_output(op_folder);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
    if(worker_comm.get_rank() == 0) {
      paracel::str_type key = "worker_sz";
      (ps_obj->kvm[clock_server]).push_int(key, worker_comm.get_size());
    }
    worker_comm.sync();
  }


  virtual ~paralg() {
    if(ps_obj) {
      delete ps_obj;
    }
  }
  
  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
  	parser_type & parser,
	const paracel::str_type & pattern = "linesplit",
	bool mix_flag = false) {
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.load();
    return lines;
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
	const paracel::str_type & pattern = "linesplit",
	bool mix_flag = false) {
    parser_type parser;
    return paralg::paracel_load(fn, parser, pattern, mix_flag);	
  }

  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			paracel::dict_type<size_t, int> & degree_map,
			paracel::dict_type<size_t, int> & col_degree_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false) {
    // TODO: check pattern 
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.load();
    // create graph 
    ld.create_graph(lines, grp, row_map, col_map, degree_map, col_degree_map);
  }

  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false) {
    paracel::dict_type<size_t, int> degree_map, col_degree_map;
    return paralg::paracel_load_as_graph(grp, 
    					row_map, col_map, degree_map, col_degree_map, 
					fn, parser, pattern, mix_flag);
  }
  
  // simple interface
  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false) {
    return paralg::paracel_load_as_graph(grp, 
    					rm, cm, dm, col_dm,
					fn, parser, pattern, mix_flag);
  }
  
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			paracel::dict_type<size_t, int> & degree_map,
			paracel::dict_type<size_t, int> & col_degree_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false) {			
    // TODO: check pattern
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.load();
    // create sparse matrix
    ld.create_matrix(lines, blk_mtx, row_map, col_map, degree_map, col_degree_map);
  }
  
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false) {
    paracel::dict_type<size_t, int> degree_map, col_degree_map;
    return paralg::paracel_load_as_matrix(blk_mtx, 
    					row_map, col_map, degree_map, col_degree_map, 
  					fn, parser, pattern, mix_flag);
  }
  
  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false) {
    return paralg::paracel_load_as_matrix(blk_mtx, 
    					rm, cm, dm, col_dm, 
					fn, parser, pattern, mix_flag);
  }

  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false) {
			
    // TODO: check pattern
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.load();
    // create sparse matrix
    ld.create_matrix(lines, blk_dense_mtx, row_map);
  }

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false) {
    return paralg::paracel_load_as_matrix(blk_dense_mtx, rm, fn, parser, pattern, mix_flag);	
  }

  // put where you want to control iter with ssp
  void iter_commit() {
    paracel::str_type clock_key;
    if(limit_s == 0) {
      clock_key = "client_clock_0";
    } else {
      clock_key = "client_clock_" + std::to_string(clock % limit_s);
    }
    ps_obj->kvm[clock_server].incr_int(paracel::str_type(clock_key), 1); // value 1 is not important
    clock += 1;
    if(clock == total_iters) {
      ps_obj->kvm[clock_server].incr_int(paracel::str_type("worker_sz"), -1);
    }
  }

  bool paracel_register_update(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    load_update_f(file_name, func_name);
    auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_update(file_name, func_name);
    }
    return r;
  }
  
  bool paracel_register_bupdate(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    //local_update_f(file_name, func_name);
    auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_bupdate(file_name, func_name);
    }
    return r;
  }
  
  bool paracel_register_read_special(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_pullall_special(file_name, func_name);
    }
    return r;
  }
  
  bool paracel_register_remove_special(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_remove_special(file_name, func_name);
    }
    return r;
  }

  template <class V>
  bool paracel_read(const paracel::str_type & key, V & val, int replica_id = -1) {
    if(ssp_switch) {
      if(clock == 0 || clock == total_iters) { // check total_iters for last pull
        cached_para[key] = boost::any_cast<V>(ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key));
	val = boost::any_cast<V>(cached_para[key]);
      } else if(stale_cache + limit_s > clock) {
        // cache hit
	val = boost::any_cast<V>(cached_para[key]);
      } else {
        // cache miss
	// pull from server until leading slowest less than s clocks
	while(stale_cache + limit_s < clock) {
	  stale_cache = ps_obj->kvm[clock_server].pull_int(paracel::str_type("server_clock"));
	}
	cached_para[key] = boost::any_cast<V>(ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key));
	val = boost::any_cast<V>(cached_para[key]);
      }
      return true;
    }
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull(key, val); 
  }
  
  template <class V>
  V paracel_read(const paracel::str_type & key, int replica_id = -1) {
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key);
  }

  // TODO
  void paracel_readall() {}

  // TODO
  void paracel_read_topk() {}

  template <class V>
  bool paracel_write(const paracel::str_type & key, const V & val, bool replica_flag = true) {
    auto indx = ps_obj->p_ring->get_server(key);
    if(ssp_switch) {
      cached_para[key] = boost::any_cast<V>(val);
    }
    return (ps_obj->kvm[indx]).push(key, val);
  }
  
  bool paracel_write(const paracel::str_type & key, const char* val, bool replica_flag = true) {
    paracel::str_type v = val;
    return paralg::paracel_write(key, v); 
  }
  
  template <class V>
  void paracel_update(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    if(ssp_switch) {
      if(!update_f) {
        // load default updater
	load_update_f("/mfs/user/wuhong/paracel/lib/default.so", "default_incr_d");
      }
      V val = boost::any_cast<V>(cached_para[key]);
      // pack<V> val to v & delta to d
      paracel::packer<V> pk1(val), pk2(delta);
      paracel::str_type v, d; pk1.pack(v); pk2.pack(d);
      auto nv = update_f(v, d);
      // unpack<V> nv
      V nval = pk1.unpack(nv);
      cached_para[key] = boost::any_cast<V>(nval);
    }
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key, delta);
  }

  void paracel_update(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_update(key, d);
  }
  
  template <class V>
  void paracel_bupdate(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    int indx = ps_obj->p_ring->get_server(key);
    ps_obj->kvm[indx].bupdate(key, delta);
    if(ssp_switch) {
      // update local cache
      cached_para[key] = boost::any_cast<V>(ps_obj->kvm[indx].pull<V>(key));
    }
  }

  void paracel_bupdate(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_bupdate(key, d);
  }

  // set invoke cnts
  void set_total_iters(int n) {
    total_iters = n;
  }

  inline int get_worker_id() {
    return worker_comm.get_rank();
  }
  
  inline int get_worker_size() {
    return worker_comm.get_size();
  }

  void sync() {
    worker_comm.sync();
  }

  paracel::Comm get_comm() {
    return worker_comm;
  }
 
  boost::any get_cache() {
    return cached_para;
  }

  template <class V>
  V get_cache(const paracel::str_type & key) {
    return boost::any_cast<V>(cached_para[key]);
  }
  
  // TODO
  bool paracel_contains(const paracel::str_type & key) {}

  template <class V>
  paracel::str_type dump_line_as_vector() {}

  template <class V>
  void dump_vector(const paracel::list_type<V> & data,
  		const paracel::dict_type<size_t, paracel::str_type> & id_map,
		const paracel::str_type & filename = "result_",
  		const paracel::str_type & sep = ",",
		bool merge = false) {}

  template <class V>
  void dump_vector(const paracel::list_type<V> & data, 
  		const paracel::str_type & filename = "result_",
  		const paracel::str_type & sep = ",", bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()));
    for(int i = 0; i < (int)data.size() - 1; ++i) {
      os << std::to_string(data[i]) << sep;
    }
    os << std::to_string(data[data.size() - 1]) << '\n';
    os.close();
  }

  virtual void solve() {}

private:
  class parasrv {
    
    using l_type = paracel::list_type<paracel::kvclt>;
    using dl_type = paracel::list_type<paracel::dict_type<paracel::str_type, paracel::str_type> >; 
  
    public:
      parasrv(paracel::str_type hosts_dct_str) {
        // init dct_lst
        dct_lst = paracel::get_hostnames_dict(hosts_dct_str);
        // init srv_sz
        srv_sz = dct_lst.size();
        // init kvm
        for(auto & srv : dct_lst) {
          paracel::kvclt kvc(srv["host"], srv["ports"]);
          kvm.push_back(std::move(kvc));
        }
        // init servers
        for(auto i = 0; i < srv_sz; ++i) {
          servers.push_back(i);
        }
        // init hashring
        p_ring = new paracel::ring<int>(servers);
      }

      virtual ~parasrv() {
        delete p_ring;
      }

    public:
      dl_type dct_lst;
      int srv_sz = 1;
      l_type kvm;
      paracel::list_type<int> servers;
      paracel::ring<int> *p_ring;
  }; // nested class definition

private:
  int nworker = 1;
  int rounds = 1;
  int stale_cache, clock, total_iters;
  int clock_server = 0;
  paracel::Comm worker_comm;
  parasrv *ps_obj;
  paracel::str_type output;
  paracel::dict_type<size_t, paracel::str_type> rm;
  paracel::dict_type<size_t, paracel::str_type> cm;
  paracel::dict_type<size_t, int> dm;
  paracel::dict_type<size_t, int> col_dm;
  paracel::dict_type<paracel::str_type, paracel::str_type> keymap;
  paracel::dict_type<paracel::str_type, boost::any> cached_para;
  int limit_s = 0;
  bool ssp_switch = false;
  paracel::update_result update_f;
};

} // namespace paracel

#endif
