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

#include <functional>

#include <boost/filesystem.hpp>
#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>

#include "paracel_types.hpp"
#include "ps.hpp"
#include "utils.hpp"
#include "client.hpp"
#include "graph.hpp"
#include "load.hpp"
#include "ring.hpp"

namespace paracel {
namespace ps {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>; 

class paralg::parasrv{
  
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
        paracel::kvclt kvc(srv["node"], srv["ports"]);
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

paralg::paralg(paracel::str_type hosts_dct_str,
	paracel::Comm comm,
	paracel::str_type op_folder,
	size_t o_rounds,
	size_t o_limit_s) : 
	worker_comm(comm),
	output(op_folder),
	nworker(comm.get_size()),
	rounds(o_rounds),
	limit_s(o_limit_s) {
  ps_obj = new parasrv(hosts_dct_str);
  // create output folder
  if(worker_comm.get_rank() == 0) {
    boost::filesystem::path path(op_folder);
    if(boost::filesystem::exists(path)) {
      boost::filesystem::remove_all(path);
    }
    boost::filesystem::create_directory(path);
  }
  worker_comm.sync();
}

paralg::~paralg() {
  delete ps_obj;
}

template <class T>
paracel::list_type<paracel::str_type> 
paralg::paracel_load(const T & fn,
		parser_type & parser,
		const paracel::str_type & pattern,
		bool mix_flag) {
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  return lines;
}

template <class T>
paracel::list_type<paracel::str_type> 
paralg::paracel_load(const T & fn,
		const paracel::str_type & pattern,
		bool mix_flag) {
  parser_type parser;
  return paralg::paracel_load(fn, parser, pattern, mix_flag);
}
  
template <class T>
void paralg::paracel_load_as_graph(paracel::bigraph & grp,
			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			paracel::dict_type<size_t, int> & degree_map,
			paracel::dict_type<size_t, int> & col_degree_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern,
			bool mix_flag) {
  // TODO: check pattern 
  // load lines
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  // create graph 
  ld.create_graph(lines, grp, row_map, col_map, degree_map, col_degree_map);
}

template <class T>
void paralg::paracel_load_as_graph(paracel::bigraph & grp,
			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern,
			bool mix_flag) {
  paracel::dict_type<size_t, int> degree_map, col_degree_map;
  return paralg::paracel_load_as_graph(grp, 
  				row_map, col_map, degree_map, col_degree_map, 
				fn, parser, pattern, mix_flag);
}
  
template <class T>
void paralg::paracel_load_as_graph(paracel::bigraph & grp,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern,
			bool mix_flag) {
  return paralg::paracel_load_as_graph(grp, 
  				rm, cm, dm, col_dm,
				fn, parser, pattern, mix_flag);
}
  
template <class T>
void paralg::paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
				paracel::dict_type<size_t, paracel::str_type> & row_map,
				paracel::dict_type<size_t, paracel::str_type> & col_map,
				paracel::dict_type<size_t, int> & degree_map,
				paracel::dict_type<size_t, int> & col_degree_map,
				const T & fn, 
  				parser_type & parser,
				const paracel::str_type & pattern,
				bool mix_flag) {
  // TODO: check pattern
  // load lines
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  // create sparse matrix
  ld.create_matrix(lines, blk_mtx, row_map, col_map, degree_map, col_degree_map);
}

template <class T>
void paralg::paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
				paracel::dict_type<size_t, paracel::str_type> & row_map,
				paracel::dict_type<size_t, paracel::str_type> & col_map,
				const T & fn, 
  				parser_type & parser,
				const paracel::str_type & pattern,
				bool mix_flag) {
  paracel::dict_type<size_t, int> degree_map, col_degree_map;
  return paralg::paracel_load_as_matrix(blk_mtx, 
  				row_map, col_map, degree_map, col_degree_map, 
  				fn, parser, pattern, mix_flag);
}

template <class T>
void paralg::paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
				const T & fn, 
  				parser_type & parser,
				const paracel::str_type & pattern,
				bool mix_flag) {
  return paralg::paracel_load_as_matrix(blk_mtx, 
  				rm, cm, dm, col_dm, 
				fn, parser, pattern, mix_flag);
}

template <class T>
void paralg::paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
				paracel::dict_type<size_t, paracel::str_type> & row_map,
				const T & fn, 
  				parser_type & parser,
				const paracel::str_type & pattern,
				bool mix_flag) {
  // TODO: check pattern
  // load lines
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  // create sparse matrix
  ld.create_matrix(lines, blk_dense_mtx, row_map);
}

template <class T>
void paralg::paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
				const T & fn, 
  				parser_type & parser,
				const paracel::str_type & pattern,
				bool mix_flag) {
  return paralg::paracel_load_as_matrix(blk_dense_mtx, rm, fn, parser, pattern, mix_flag);
}

bool paralg::paracel_register_update(const paracel::str_type & file_name, 
		const paracel::str_type & func_name) {
  auto rg = ps_obj->p_ring;
  bool r = true;
  for(int i = 0; i < ps_obj->srv_sz; ++i) {
    r = r && ps_obj->kvm[i].register_update(file_name, func_name);
  }
  return r;
}

bool paralg::paracel_register_pullall_special(const paracel::str_type & file_name, 
				const paracel::str_type & func_name) {
  auto rg = ps_obj->p_ring;
  bool r = true;
  for(int i = 0; i < ps_obj->srv_sz; ++i) {
    r = r && ps_obj->kvm[i].register_pullall_special(file_name, func_name);
  }
  return r;
}

bool paralg::paracel_register_remove_special(const paracel::str_type & file_name, 
				const paracel::str_type & func_name) {
  auto rg = ps_obj->p_ring;
  bool r = true;
  for(int i = 0; i < ps_obj->srv_sz; ++i) {
    r = r && ps_obj->kvm[i].register_remove_special(file_name, func_name);
  }
  return r;
}

template <class V>
bool paralg::paracel_read(const paracel::str_type & key, V & val) {
  return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull(key, val); 
}

template <class V>
V paralg::paracel_read(const paracel::str_type & key) {
  return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key);
}

template <class V>
bool paralg::paracel_write(const paracel::str_type & key, const V & val) {
  return ps_obj->kvm[ps_obj->p_ring->get_server(key)].push(key, val);
}

bool paralg::paracel_write(const paracel::str_type & key, const char* val) {
  paracel::str_type v = val;
  return paralg::paracel_write(key, v);
}

template <class V>
void paralg::paracel_update(const paracel::str_type & key, const V & delta) {
  ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key, delta);
}

void paralg::paracel_update(const paracel::str_type & key, const char* delta) {
  paracel::str_type d = delta;
  paralg::paracel_update(key, d);
}
  
inline size_t paralg::get_worker_id() {
  return worker_comm.get_rank();
}
  
inline size_t paralg::get_worker_size() {
  return worker_comm.get_size();
}

void paralg::sync() {
  worker_comm.sync();
}

template <class V>
paracel::str_type paralg::dump_line_as_vector() {}

template <class V>
void paralg::dump_vector(const paracel::str_type & path, 
			const paracel::dict_type<size_t, paracel::str_type> & id_map,
		  	const paracel::list_type<V> & data,
		  	const paracel::Comm & comm, 
			bool merge) {}

void paralg::solve() {}

} // namespace ps 
} // namespace paracel
