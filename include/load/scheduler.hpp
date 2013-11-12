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

#include "paracel_types.hpp"
#include "utils/comm.hpp"

namespace paracel {

class scheduler {

public:
 
  scheduler(paracel::Comm comm) : m_comm(comm) {} 
  
  scheduler(paracel::Comm comm, int master) : leader(master), m_comm(comm) {}
  
  paracel::list_type<paracel::str_type> 
  pload(paracel::deque_type< paracel::coroutine<paracel::str_type> > & loads) {
    paracel::list_type<paracel::str_type> result;
    int rk = m_comm.get_rank();
    int sz = m_comm.get_size();
    int flag = 0;
    int cnt_control = BLK_SZ;
    int cnt = cnt_control - 1;
    int ntasks = sz * cnt_control;
    int bnd = ntasks + sz - 2;
    if(rk == leader) {
      // load tasks [0, BLK_SZ - 1]
      for(int i = 0; i < BLK_SZ; ++i) {
        // loading lines
	while(loads[i]) {
	  auto line = loads[i].get();
	  result.push_back(line);
	  loads[i]();
	}
      }
    }
    if(rk != leader) {
      while(1) {
        if(flag) break;
	if(cnt == ntasks - 1) break;
	m_comm.send(rk, leader, 2013);
	m_comm.recv(cnt, leader, 2013);
	m_comm.recv(flag, leader, 2013);
	if(!flag) {
	  // loading lines
	  while(loads[cnt]) {
	    auto line = loads[cnt].get();
	    result.push_back(line);
	    loads[cnt]();
	  }
	}
      } // end of while
    } else {
      while(cnt_control < bnd) {
	cnt_control += 1;
	int tmp;
	auto status = m_comm.recv(tmp, any_source, any_tag);
	if(!flag) cnt += 1;
	int src = m_comm.get_source(status);
	m_comm.send(cnt, src, 2013);

	if( (cnt == ntasks - 1) && (!flag) ) {
	  m_comm.send(flag, src, 2013);
	  flag = 1;
	} else {
	  m_comm.send(flag, src, 2013);
	}
      } // end of while
    } // end of if-else
    return result;
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
