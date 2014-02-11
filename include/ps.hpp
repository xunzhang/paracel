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

#include <functional>

#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>

#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "graph.hpp"

namespace paracel {
namespace ps {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

class paralg {
public:
  paralg(paracel::str_type hosts_dct_str, 
  	paracel::Comm comm,
	paracel::str_type op_folder,
	size_t o_rounds,
	size_t o_limit_s = 0); 

  virtual ~paralg();

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
  	parser_type & parser,
	const paracel::str_type & pattern = "linesplit",
	bool mix_flag = false);

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
	const paracel::str_type & pattern = "linesplit",
	bool mix_flag = false);

  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			paracel::dict_type<size_t, int> & degree_map,
			paracel::dict_type<size_t, int> & col_degree_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false);

  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false);
  
  // simple interface
  template <class T>
  void paracel_load_as_graph(paracel::bigraph & grp,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fmap",
			bool mix_flag = false);
  
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			paracel::dict_type<size_t, int> & degree_map,
			paracel::dict_type<size_t, int> & col_degree_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false);
  
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			paracel::dict_type<size_t, paracel::str_type> & col_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false);
  
  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false);

  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
  			paracel::dict_type<size_t, paracel::str_type> & row_map,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false);

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
			const T & fn, 
			parser_type & parser,
			const paracel::str_type & pattern = "fsmap",
			bool mix_flag = false);
  
  bool paracel_register_update(const paracel::str_type & file_name, const paracel::str_type & func_name);
  
  bool paracel_register_pullall_special(const paracel::str_type & file_name, const paracel::str_type & func_name);
  
  bool paracel_register_remove_special(const paracel::str_type & file_name, const paracel::str_type & func_name);

  template <class V>
  bool paracel_read(const paracel::str_type & key, V & val);
  
  template <class V>
  V paracel_read(const paracel::str_type & key);
  
  template <class V>
  bool paracel_write(const paracel::str_type & key, const V & val);
  
  bool paracel_write(const paracel::str_type & key, const char* val);
  
  template <class V>
  void paracel_update(const paracel::str_type & key, const V & delta);

  void paracel_update(const paracel::str_type & key, const char* delta);

  inline size_t get_worker_id();
  
  inline size_t get_worker_size();

  void sync();
  
  template <class V>
  paracel::str_type dump_line_as_vector();

  template <class V>
  void dump_vector(const paracel::str_type & path, 
                  const paracel::dict_type<size_t, paracel::str_type> & id_map,
		  const paracel::list_type<V> & data,
		  const paracel::Comm & comm, 
		  bool merge = false);

  virtual void solve();

private:
  class parasrv;
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
};

} // namespace ps
} // namespace paracel

#endif
