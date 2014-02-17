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

#include <fstream>
#include <algorithm>
#include <functional>

#include <boost/filesystem.hpp>

#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>

#include "paracel_types.hpp"
#include "client.hpp"
#include "utils.hpp"
#include "graph.hpp"
#include "load.hpp"
#include "ring.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

class paralg {

public:
  
  // interface for direct usage
  paralg(paracel::Comm comm,
  	paracel::str_type op_folder,
  	size_t o_rounds) : 
		worker_comm(comm), 
		output(op_folder), 
		rounds(o_rounds) {
    init_output(op_folder);
  }

  paralg(paracel::str_type hosts_dct_str, 
  	paracel::Comm comm,
	paracel::str_type op_folder,
	size_t o_rounds,
	size_t o_limit_s = 0) :
	worker_comm(comm),
	output(op_folder),
	nworker(comm.get_size()),
	rounds(o_rounds),
	limit_s(o_limit_s) {
    ps_obj = new parasrv(hosts_dct_str);
    init_output(op_folder);
    worker_comm.sync();
  }

  virtual ~paralg() {
    if(ps_obj) {
      delete ps_obj;
    }
  }
  
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
  
  bool paracel_register_update(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_update(file_name, func_name);
    }
    return r;
  }
  
  bool paracel_register_bupdate(const paracel::str_type & file_name, const paracel::str_type & func_name) {
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
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull(key, val); 
  }
  
  template <class V>
  V paracel_read(const paracel::str_type & key, int replica_id = -1) {
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key);
  }
  
  template <class V>
  bool paracel_write(const paracel::str_type & key, const V & val, bool replica_flag = true) {
    auto indx = ps_obj->p_ring->get_server(key);
    return (ps_obj->kvm[indx]).push(key, val);
  }
  
  bool paracel_write(const paracel::str_type & key, const char* val, bool replica_flag = true) {
    paracel::str_type v = val;
    return paralg::paracel_write(key, v); 
  }
  
  template <class V>
  void paracel_update(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key, delta);
  }

  void paracel_update(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_update(key, d);
  }
  
  template <class V>
  void paracel_bupdate(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].bupdate(key, delta);
  }

  void paracel_bupdate(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_bupdate(key, d);
  }

  inline size_t get_worker_id() {
    return worker_comm.get_rank();
  }
  
  inline size_t get_worker_size() {
    return worker_comm.get_size();
  }

  void sync() {
    worker_comm.sync();
  }

  paracel::Comm get_comm() {
    return worker_comm;
  }
  
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
      size_t srv_sz = 1;
      l_type kvm;
      paracel::list_type<int> servers;
      paracel::ring<int> *p_ring;
  }; // nested class definition

private:
  size_t nworker = 1;
  size_t rounds = 0;
  size_t limit_s = 0;
  paracel::Comm worker_comm;
  parasrv *ps_obj;
  paracel::str_type output;
  paracel::dict_type<size_t, paracel::str_type> rm;
  paracel::dict_type<size_t, paracel::str_type> cm;
  paracel::dict_type<size_t, int> dm;
  paracel::dict_type<size_t, int> col_dm;
  paracel::dict_type<paracel::str_type, paracel::str_type> keymap;
};

} // namespace paracel

#endif
