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
 *
 */
#ifndef FILE__92d02472_9aee_fd8f_021c_3b914917cd9f__HPP 
#define FILE__92d02472_9aee_fd8f_021c_3b914917cd9f__HPP

#include <mpi.h>

#include "paracel_types.hpp"

namespace paracel {

class main_env {
public:
  main_env(int argc, char *argv[]) { MPI_Init (&argc, &argv); }
  ~main_env() { MPI_Finalize(); }
};

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
 
  // api for paracel is_comm_builtin type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  send(const T & data, int dest, int tag);
  
  // api for paracel is_comm_container type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  send(const T & data, int dest, int tag);
  
  // impl for is_comm_container type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  send(const T & data, int sz, int dest, int tag);
  
  // api for paracel is_comm_builtin type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Request>
  isend(const T & data, int dest, int tag);

  // api for paracel is_comm_container type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Request>
  isend(const T & data, int dest, int tag);
  
  // impl for is_comm_container type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, MPI_Request>
  isend(const T & data, int sz, int dest, int tag);

  // api for paracel is_comm_builtin type recv
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  recv(T & data, int src, int tag);
  
  // api for paracel is_comm_container type recv
  // notice: data size here must be defined before function call
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  recv(T & data, int src, int tag);

  // impl for paracel is_comm_container type recv
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  recv(T & data, int sz, int src, int tag);
  
  void wait(MPI_Request & req);

  // api for paracel is_comm_builtin type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag);

  // api for paracel is_comm_container type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag);

  // impl for paracel is_comm_container type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  sendrecv(const T &sdata, int sz, T & rdata, int sto, int stag, int rfrom, int rtag);

  // api for paracel is_comm_builtin type bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  bcast(T & data, int master);

  // api for paracel is_comm_container type bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  bcast(T & data, int master);

  // impl for paracel is_comm_container type cast
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  bcast(T & data, int sz, int master);

  // api for paracel bcastring
  template <class T, class F>
  void bcastring(const T & data, F & func);

  // api for paracel is_comm_builtin type alltoall
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_builtin<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf);

  // api for paracel is_comm_container type alltoall
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf);

  // api for paracel alltoallring
  template <class T, class F>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoallring(const T & sbuf, T & rbuf, F & func);

  // api for paracel is_comm_builtin type allreduce
  template <class T, class F>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  allreduce(T & data, F & func);

  // api for paracel is_comm_container type allreduce
  template <class T, class F>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  allreduce(T & data, F & func);

  // impl for paracel is_comm_container type allreduce
  template <class T, class F>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  allreduce(T & data, int sz, F & func);

private:
  MPI_Comm m_comm;
  int m_rk, m_sz;
};

} // namespace paracel

#endif
