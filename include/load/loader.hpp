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

#include <functional>

#include "paracel_types.hpp"
#incude "load/scheduler.hpp"

namespace paracel {

typedef std::function< paracel::list_type<paracel::str_type>(paracel::str_type) parser_type;

template <class T>
class loader {

public:
  
  loader(T fns, paracel::Comm comm) : filenames(fns), m_comm(comm) {};
  
  loader(T fns, paracel::Comm comm, parser_type f, paracel::str_type pt) : filenames(fns), m_comm(comm) parserfunc(f), pattern(pt){};
  
  loader(T fns, paracel::Comm comm, parser_type f,  paracel::str_type pt, bool flag) : filenames(fns), m_comm(comm) parserfunc(f), pattern(pt), mix(flag) {};
  
  void graph_load(paracel::dict_type<size_t, paracel::str_type> & rm, 
  		paracel::dict_type<size_t, paracel::str_type> & cm,
      		paracel::dict_type<size_t, int> & dm,
      		paracel::dict_type<size_t, int> & col_dm) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto fname_lst = paracel::expand(filenames);
    auto loads = paracel::files_partition(fname_lst, m_comm.get_size());
    auto linelst = scheduler.schedule_load(loads);
    auto result = scheduler.lines_organize(linelst, parserfunc);
    auto stf = scheduler.exchange(result);
    paracel::list_type<std::tuple<size_t, size_t, double> > stf_new;
    scheduler.index_mapping(stf, stf_new, rm, cm, dm, col_dm);
  }

  void load(paracel::dict_type<size_t, paracel::str_type> & rm,
  	paracel::dict_type<size_t, paracel::str_type> & cm) {

    paracel::dict_type<size_t, int> dm;
    paracel::dict_type<size_t, int> col_dm;
    graph_load(blk_mtx, rm, cm, dm, col_dm);
  }

private:
  T filenames;
  paracel::Comm m_comm;
  paracel::str_type pattern = "fmap";
  bool mix = false;
  std::function< paracel::list_type<paracel::str_type>(paracel::str_type) parserfunc;
};

} // namespace paracel

#endif
