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
#ifndef FILE_8199e926_217a_8205_0832_1df88970b15d_HPP
#define FILE_8199e926_217a_8205_0832_1df88970b15d_HPP

#include <cstdlib>
#include <mutex>
#include <functional>
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "utils/decomp.hpp"
#include "utils/ext_utility.hpp"

namespace paracel {

std::mutex mutex;

typedef paracel::deque_type<paracel::coroutine<paracel::str_type> > schedule_load_para_type;
typedef paracel::list_type<paracel::triple_type> lt_type;
typedef paracel::list_type<paracel::list_type<paracel::triple_type> > llt_type;

auto tmp_parser = [](const paracel::str_type & a) { return a; };

class scheduler {

public:
  scheduler(paracel::Comm comm) : m_comm(comm) { dim_init(); } 

  scheduler(paracel::Comm comm, std::string pt = "fmap", bool flag = false) : mix(flag), pattern(pt), m_comm(comm) { dim_init(); }
  
  void dim_init();
   
  paracel::list_type<paracel::str_type> schedule_load(schedule_load_para_type & loads);

  template <class A, class B>
  inline size_t h(A & i, B & j, int & nx, int & ny) { 
    paracel::hash_type<A> hf1; 
    paracel::hash_type<B> hf2;
    size_t r = (hf1(i) % nx) * ny + hf2(j) % ny;
    return r;
  }

  template <class F = std::function< paracel::list_type<paracel::str_type>(paracel::str_type) > >
  llt_type lines_organize(const paracel::list_type<paracel::str_type> & lines,
      F && parser_func = tmp_parser) {

    llt_type line_slot_lst(m_comm.get_size());
    paracel::str_type delimiter("[:| ]*");
    for(auto & line : lines) { 
      auto stf = parser_func(line);
      if(stf.size() == 2) {
        // bfs or part of fset case
	// ['a', 'b'] or ['a', 'b:0.2']
	auto tmp = paracel::str_split(stf[1], delimiter);
	if(tmp.size() == 1) {
	  paracel::triple_type tpl(stf[0], stf[1], 1.);
	  line_slot_lst[h(stf[0], stf[1], npx, npy)].push_back(tpl);
	} else {
	  paracel::triple_type tpl(stf[0], tmp[0], std::stod(tmp[1]));
	  line_slot_lst[h(stf[0], tmp[0], npx, npy)].push_back(tpl);
	}
      } else if(mix) {
        // fset case
	// ['a', 'b', 'c'] or ['a', 'b|0.2', 'c|0.4'], but ['a', '0.2', '0.4'] is not supported here
        for(size_t i = 1; i < stf.size(); ++i) {
	  auto item = stf[i];
	  auto tmp = paracel::str_split(item, delimiter);
	  if(tmp.size() == 1) {
	    paracel::triple_type tpl(stf[0], item, 1.);
	    line_slot_lst[h(stf[0], item, npx, npy)].push_back(tpl);
	  } else {
	    paracel::triple_type tpl(stf[0], tmp[0], std::stod(tmp[1]));
	    line_slot_lst[h(stf[0], tmp[0], npx, npy)].push_back(tpl);
	  }
	} // end of for
      } else {
	if(stf.size() != 3) { throw std::runtime_error("Paracel error in lines_organize: fmt of input files not supported"); }
        // fsv case
        paracel::triple_type tpl(stf[0], stf[1], std::stod(stf[2]));
	line_slot_lst[h(stf[0], stf[1], npx, npy)].push_back(tpl);
      } // end of if
    } // end of for
    return line_slot_lst;
  }

  lt_type exchange(llt_type & line_slot_lst) {
    llt_type recv_lsl;
    lt_type stf;
    m_comm.alltoall(line_slot_lst, recv_lsl);
    for(auto & lst : recv_lsl) {
      for(auto & tpl : lst) {
	stf.push_back(tpl);
      }
    }
    return stf;
  }
 
  // dm and col_dm only support fmap
  void index_mapping(const lt_type & slotslst, 
      paracel::list_type<std::tuple<size_t, size_t, double> > & stf, 
      paracel::dict_type<size_t, paracel::str_type> & rm,
      paracel::dict_type<size_t, paracel::str_type> & cm,
      paracel::dict_type<size_t, int> & dm,
      paracel::dict_type<size_t, int> & col_dm) {
    
    int rk = m_comm.get_rank();
    int rowcolor = rk / npy;
    int colcolor = rk % npy;
    auto col_comm = m_comm.split(colcolor);
    auto row_comm = m_comm.split(rowcolor);

    paracel::list_type<paracel::str_type> rows, cols;
    for(auto & tpl : slotslst) {
      rows.push_back(std::get<0>(tpl));
      cols.push_back(std::get<1>(tpl));
    }
    
    paracel::set_type<paracel::str_type> new_rows, new_cols;
    auto union_func1 = [&] (paracel::list_type<paracel::str_type> tmp) {
      for(auto & item : tmp) { 
        new_rows.insert(item); 
      }
    };
    auto union_func2 = [&] (paracel::list_type<paracel::str_type> tmp) {
      for(auto & item : tmp) { 
        new_cols.insert(item); 
      }
    };
    row_comm.bcastring(rows, union_func1);
    col_comm.bcastring(cols, union_func2);
    
    paracel::dict_type<paracel::str_type, size_t> rev_rm, rev_cm;
    size_t indx = 0;
    for(auto & item : new_rows) {
      rm[indx] = item;
      rev_rm[item] = indx;
      indx += 1;
    }
    indx = 0;
    for(auto & item : new_cols) {
      cm[indx] = item;
      rev_cm[item] = indx;
      indx += 1;
    }

    for(auto & tpl : slotslst) {
      std::tuple<size_t, size_t, double> tmp;
      std::get<0>(tmp) = rev_rm[std::get<0>(tpl)];
      std::get<1>(tmp) = rev_cm[std::get<1>(tpl)];
      std::get<2>(tmp) = std::get<2>(tpl);
      stf.push_back(tmp);
    }

    if (pattern == "fmap") {
    // cal dm
    // little tricky: default stl map sort is equal to stl set
    auto deg = paracel::sort_and_cnt(rows);
    indx = 0;
    for(auto & item : deg) {
      dm[indx] = item;
      indx += 1;
    }
    
    // cal col_dm
    paracel::dict_type<size_t, int> reduce_map;
    for(auto & tpl : stf) {
      auto key = std::get<1>(tpl);
      if(reduce_map.find(key) == reduce_map.end()) {
        reduce_map[key] = 1;
      } else {
        reduce_map[key] += 1;
      }
    }
    
    auto union_func3 = [&] (paracel::dict_type<size_t, int> tmp) {
      for(auto & kv : tmp) {
        auto key = kv.first;
	auto val = kv.second;
	if(col_dm.find(key) == col_dm.end()) {
	  col_dm[key] = val;
	} else {
	  col_dm[key] += val;
	}
      }
    };
    m_comm.bcastring(reduce_map, union_func3);
    } // end of if 
  } // index_mapping
  
private:
  int randint(int l, int u) {
    srand((unsigned)time(NULL));
    return l + rand() % (u - l + 1);
  }

  void elect() { leader = randint(0, m_comm.get_size() - 1); }

private:
  int leader = 0;
  bool mix;
  paracel::str_type pattern; 
  paracel::Comm m_comm;
  int npx;
  int npy;
};

} // namespace parael
#endif
