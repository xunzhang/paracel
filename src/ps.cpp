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
	size_t n_worker,
	size_t o_rounds,
	size_t o_limit_s) : 
	worker_comm{comm},
	nworker(n_worker),
	rounds(o_rounds),
	limit_s(o_limit_s) {
  ps_obj = new parasrv(hosts_dct_str);
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
  // TODO: check pattern 
  // load lines
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  paracel::dict_type<size_t, int> degree_map, col_degree_map;
  // create graph 
  ld.create_graph(lines, grp, row_map, col_map, degree_map, col_degree_map);
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
  // TODO: check pattern
  // load lines
  paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
  paracel::list_type<paracel::str_type> lines = ld.load();
  // create sparse matrix
  paracel::dict_type<size_t, int> degree_map, col_degree_map;
  ld.create_matrix(lines, blk_mtx, row_map, col_map, degree_map, col_degree_map);
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

template <class V>
bool paracel_read(const paracel::str_type & key, V & val) {}

} // namespace ps 
} // namespace paracel
