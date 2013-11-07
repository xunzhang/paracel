/**
 * Copyright (c) 2013, Douban Inc. 
 *   All rights reserved. 
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
 *
 * Authors: 
 * Hong Wu <xunzhangthu@gmail.com>
 * Changsheng Jiang <jiangzuoyan@gmail.com>
 *
 */
#ifndef FILE__92d02472_9aee_fd8f_021c_3b914917cd9f__HPP 
#define FILE__92d02472_9aee_fd8f_021c_3b914917cd9f__HPP

#include <mpi.h>

namespace paracel {

class Comm {

public:

  Comm(MPI_Comm comm = MPI_COMM_WORLD);

  Comm(const Comm &);

  Comm(Comm &&);

  ~Comm();

  Comm& operator=(const Comm &);

  Comm& operator=(Comm &&);

  void init(MPI_Comm comm);
  
  inline size_t get_size() const { return m_sz; }
  
  inline size_t get_rank() const { return m_rk; }

  inline MPI_Comm get_comm() const { return m_comm; }

  inline void sync() { MPI_Barrier(m_comm); }
  
  template <class T>
  void send(const T &, int, int);

  template <class T>
  MPI_Request& isend(const T &, int, int);

  template <class T>
  T& recv(int, int, MPI_Status);
  
  template <class T>
  T& sendrecv(const T &, int, int, int, int);

  template <class T, class F>
  void bcastring(const T &, F &);
  
  template <class T>
  T& alltoall(const T &);

private:
  MPI_Comm m_comm;
  int m_rk, m_sz;
};

} // namespace paracel

#endif
