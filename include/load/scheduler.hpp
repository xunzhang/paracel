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

namespace paracel {

std::mutex mutex;

typedef paracel::deque_type< paracel::coroutine<paracel::str_type> > schedule_load_para_type;
typedef paracel::list_type< paracel::list_type<paracel::str_type> > lo_ret_type;

auto tmp_parser = [](const paracel::str_type & a) { return a; }

class scheduler {

public:
  scheduler(paracel::Comm comm) : m_comm(comm) {} 

  scheduler(paracel::Comm comm, int master) : leader(master), m_comm(comm) {}
  
  paracel::list_type<paracel::str_type> schedule_load(schedule_load_para_type & loads);

  template <class A, class B>
  inline size_t h(A & i, B & j, int & nx, int & ny) { 
    paracel::hash<A> hf1; paracel::hash<B> hf2;
    size_t r = (hf1(i) % ny) * nx + hf2(j) % ny;
    return r;
  }

/*
  template <class F = std::function<paracel::str_type<const paracel::str_type> > >
  lo_ret_type 
  lines_organize(paracel::list_type<paracel::str_type> && lines,
      F && func = tmp_parser, 
      paracel::str_type & pattern = "fmap", 
      bool mix = false) {

    int np = m_comm.get_size();
    int npx, npy;
    npfactx(np, npx, npy);
    if(pattern == "fsmap") npfact2d(np, npx, npy);
    if(pattern == "smap") npfacty(np, npx, npy);
    paracel::list_type< paracel::list_type<paracel::triple_type> > line_slot_lst(np);
    for(auto & line : lines) {
    }
    return line_slot_lst;
  }
*/

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
