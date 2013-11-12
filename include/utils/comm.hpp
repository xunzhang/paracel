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
 * Authors: 
 * Hong Wu <xunzhangthu@gmail.com>
 *
 */
#ifndef FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP 
#define FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP

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

  int get_source(MPI_Status & stat) { return stat.MPI_SOURCE; }
 
  // api for paracel is_comm_builtin type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  send(const T & data, int dest, int tag) {
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Send((void *)&data, 1, dtype, dest, tag, m_comm);
  }
  
  // api for paracel is_comm_container type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  send(const T & data, int dest, int tag) {
    int sz = (int)data.size();
    send(data, sz, dest, tag);
  }
  
  // impl for is_comm_container type send
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  send(const T & data, int sz, int dest, int tag) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Send((void *)&data[0], sz, dtype, dest, tag, m_comm);
  }
  
  // api for paracel is_comm_builtin type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Request>
  isend(const T & data, int dest, int tag) {
    MPI_Request req;
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Isend((void *)&data, 1, dtype, dest, tag, m_comm, &req);
    return req;
  }

  // api for paracel is_comm_container type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Request>
  isend(const T & data, int dest, int tag) {
    int sz = (int)data.size();
    return isend(data, sz, dest, tag);
  }
  
  // impl for is_comm_container type isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, MPI_Request>
  isend(const T & data, int sz, int dest, int tag) {
    MPI_Request req;
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Isend((void *)&data[0], sz, dtype, dest, tag, m_comm, &req);
    return req;
  }

  // api for paracel is_comm_builtin type recv
  // design tips:
  // if return recv data, no template var in parameter
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    MPI_Status stat;
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Recv(&data, 1, dtype, src, tag, m_comm, &stat);
    return stat;
  }
  
  // api for paracel is_comm_container type recv
  // notice: data size here must be defined before function call
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    int sz = (int)data.size();
    return recv(data, sz, src, tag);
  }

  // impl for paracel is_comm_container type recv
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, MPI_Status>
  recv(T & data, int sz, int src, int tag) {
    MPI_Status stat;
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Recv((void *)&data[0], sz, dtype, src, tag, m_comm, &stat);
    return stat;
  }
  
  void wait(MPI_Request & req) {
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  }

  // api for paracel is_comm_builtin type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
    //MPI_Status stat;
    //MPI_Datatype dtype = paracel::datatype<T>();
    //MPI_Sendrecv((void *)&sdata, 1, dtype, sto, stag, (void *)&rdata, 1, dtype, rfrom, rtag, m_comm, &stat);
    MPI_Request req = isend(sdata, sto, stag);
    recv(rdata, rfrom, rtag);
    wait(req);
  }

  // api for paracel is_comm_container type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
    int sz = (int)sdata.size();
    rdata.resize(sz);
    sendrecv(sdata, sz, rdata, sto, stag, rfrom, rtag);
  }

  // impl for paracel is_comm_container type sendrecv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  sendrecv(const T &sdata, int sz, T & rdata, int sto, int stag, int rfrom, int rtag) {
    MPI_Request req = isend(sdata, sz, sto, stag);
    recv(rdata, sz, rfrom, rtag);
    wait(req);
  }

  // api for paracel is_comm_builtin type bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  bcast(T & data, int master) {
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Bcast((void *)&data, 1, dtype, master, m_comm);
  }

  // api for paracel is_comm_container type bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  bcast(T & data, int master) {
    int sz = (int)data.size();
    bcast(data, sz, master);
  }

  // impl for paracel is_comm_container type cast
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  bcast(T & data, int sz, int master) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Bcast((void *)&data[0], sz, dtype, master, m_comm);
  }

  // api for paracel bcastring
  template <class T, class F>
  void bcastring(const T & data, F & func) {
    func(data);
    if(m_sz == 1) return;
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      T rbuf;
      sendrecv(data, rbuf, t, 2013, f, 2013, m_comm);
      func(rbuf);
    }
  }

  // api for paracel is_comm_builtin type alltoall
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_builtin<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Alltoall((void *)&sbuf[0], 1, dtype, (void *)&rbuf[0], 1, dtype, m_comm);
  }

  // api for paracel is_comm_container type alltoall
  // MPI_Alltoallv is really awful
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf) {
    rbuf.resize(sbuf.size());
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      auto tmpr = rbuf[0]; tmpr.clear();
      sendrecv(sbuf[t], tmpr, t, 2013, f, 2013);
      rbuf[t].insert(rbuf[t].end(), tmpr.begin(), tmpr.end());
    }
  }

  // api for paracel alltoallring
  template <class T, class F>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoallring(const T & sbuf, T & rbuf, F & func) {
    rbuf.resize(sbuf.size());
    func(sbuf[m_rk]);
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      auto tmpr = rbuf[0]; tmpr.clear();
      sendrecv(sbuf[t], tmpr, t, 2013, f, 2013);
      func(tmpr);
    }
  }

  // api for paracel is_comm_builtin type allreduce
  // TODO: abstract MPI_SUM with func
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  allreduce(T & data) {
    MPI_Datatype dtype = paracel::datatype<T>();
    T tmp;
    MPI_Allreduce((void *)&data, (void *)&tmp, 1, dtype, MPI_SUM, m_comm);
    data = tmp;
  }

  // api for paracel is_comm_container type allreduce
  // TODO: abstract MPI_SUM with func
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  allreduce(T & data) {
    int sz = (int)data.size();
    allreduce(data, sz);
  }

  // impl for paracel is_comm_container type allreduce
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  allreduce(T & data, int sz) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    T tmp;
    tmp.resize(data.size());
    MPI_Allreduce((void *)&data[0], (void *)&tmp[0], sz, dtype, MPI_SUM, m_comm);
    for(int i = 0; i < data.size(); ++i) {
      data[i] = tmp[i];
    }		    
  }

private:
  MPI_Comm m_comm;
  int m_rk, m_sz;
};

} // namespace paracel

#endif
