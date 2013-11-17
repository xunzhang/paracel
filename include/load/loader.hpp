/**
 * Copyright (c) 2013, Douban Inc. 
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
#ifndef FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP
#define FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP

#include <iostream>
#include <functional>

#include <boost/variant.hpp>

#include "paracel_types.hpp"
#include "load/scheduler.hpp"
#include "load/partition.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

using var_ret_type = boost::variant<paracel::list_type<paracel::str_type>, bool>;

template <class T = paracel::str_type>
class loader {

public:
  
  loader(T fns, paracel::Comm comm) : filenames(fns), m_comm(comm) {};
  
  loader(T fns, paracel::Comm comm, parser_type f, paracel::str_type pt) : filenames(fns), m_comm(comm), parserfunc(f), pattern(pt) {};
  
  loader(T fns, paracel::Comm comm, parser_type f, paracel::str_type pt, bool flag) : filenames(fns), m_comm(comm), parserfunc(f), pattern(pt), mix(flag) {};

  paracel::list_type<paracel::str_type> 
  load() {
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto fname_lst = paracel::expand(filenames);
    // generate loads
    auto loads = paracel::files_partition(fname_lst, m_comm.get_size());
    std::cout << "procs " << m_comm.get_rank() << " loads finished" << std::endl;
    // parallel loading lines
    auto linelst = scheduler.schedule_load(loads);
    std::cout << "procs " << m_comm.get_rank() << " lines got" << std::endl;
    m_comm.sync();
    return linelst;
  }

  // fmap case
  void create_matrix(const paracel::list_type<paracel::str_type> & linelst,
  		paracel::dict_type<size_t, paracel::str_type> & rm, 
		paracel::dict_type<size_t, paracel::str_type> & cm,
		paracel::dict_type<size_t, int> & dm,
		paracel::dict_type<size_t, int> & col_dm) {
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto result = scheduler.lines_organize(linelst, parserfunc);
    auto stf = scheduler.exchange(result);
    paracel::list_type<std::tuple<size_t, size_t, double> > stf_new;
    scheduler.index_mapping(stf, stf_new, rm, cm, dm, col_dm);
    // TODO: generate block sparse matrix using stf_new
  }

  // simple fmap case, fsmap case
  void create_matrix(const paracel::list_type<paracel::str_type> & linelst,
  		paracel::dict_type<size_t, paracel::str_type> & rm,
		paracel::dict_type<size_t, paracel::str_type> & cm) {
    paracel::dict_type<size_t, int> dm;
    paracel::dict_type<size_t, int> col_dm;
    create_matrix(linelst, rm, cm, dm, col_dm);
  }

  // fvec case, only support line decomposition
  void create_matrix(const paracel::list_type<paracel::str_type> & linelst,
  		paracel::dict_type<size_t, paracel::str_type> & rm) {
  }

private:
  T filenames;
  paracel::Comm m_comm;
  paracel::str_type pattern = "fmap";
  bool mix = false;
  parser_type parserfunc;
};

} // namespace paracel

#endif
