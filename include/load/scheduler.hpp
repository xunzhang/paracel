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
#include "paracel_types.hpp"
#include "utils/comm.hpp"

namespace paracel {

std::mutex mutex;

typedef paracel::deque_type< paracel::coroutine<paracel::str_type> > schedule_load_para_type;

class scheduler {

public:
  scheduler(paracel::Comm comm) : m_comm(comm) {} 

  scheduler(paracel::Comm comm, int master) : leader(master), m_comm(comm) {}
  
  paracel::list_type<paracel::str_type> schedule_load(schedule_load_para_type & loads);

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
