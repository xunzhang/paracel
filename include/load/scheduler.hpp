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
#ifndef FILE_8199e926_217a_8205_0832_1df88970b15d_HPP
#define FILE_8199e926_217a_8205_0832_1df88970b15d_HPP

#include <cstdlib>
#include <mutex>
#include <functional>
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "utils/decomp.hpp"
#include "utils/ext_str_utils.hpp"

namespace paracel {

std::mutex mutex;

typedef paracel::deque_type<paracel::coroutine<paracel::str_type> > schedule_load_para_type;
typedef paracel::list_type<paracel::triple_type> lt_type;
typedef paracel::list_type<paracel::list_type<paracel::triple_type> > llt_type;

auto tmp_parser = [](const paracel::str_type & a) { return a; };

class scheduler {

public:
  scheduler(paracel::Comm comm) : m_comm(comm) {} 

  scheduler(paracel::Comm comm, int master) : leader(master), m_comm(comm) {}
  
  paracel::list_type<paracel::str_type> schedule_load(schedule_load_para_type & loads);

  template <class A, class B>
  inline size_t h(A & i, B & j, int & nx, int & ny) { 
    paracel::hash_type<A> hf1; 
    paracel::hash_type<B> hf2;
    size_t r = (hf1(i) % nx) * ny + hf2(j) % ny;
    return r;
  }

  template <class F = std::function< paracel::list_type<paracel::str_type>(paracel::str_type) > >
  llt_type lines_organize(paracel::list_type<paracel::str_type> & lines,
      F && parser_func = tmp_parser, 
      const paracel::str_type & pattern = "fmap", 
      bool mix = false) {
    
    int npx, npy;
    int np = m_comm.get_size();
    llt_type line_slot_lst(np);
    npfactx(np, npx, npy);
    if(pattern == "fsmap") npfact2d(np, npx, npy);
    if(pattern == "smap") npfacty(np, npx, npy);
    paracel::str_type delimiter("[:| ]*");
    for(auto & line : lines) { 
      auto stf = parser_func(line);
      if(stf.size() == 2) {
        // bfs or part of fset case
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
        for(int i = 1; i < stf.size(); ++i) {
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
  
private:
  int randint(int l, int u) {
    srand((unsigned)time(NULL));
    return l + rand() % (u - l + 1);
  }

  void elect() { leader = randint(0, m_comm.get_size() - 1); }

private:
  int leader = 0;
  paracel::Comm m_comm;
};

} // namespace parael
#endif
