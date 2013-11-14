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

/**
 * a simple version, just for paracel usage
 * full version is implemented at Douban by Changsheng Jiang
 *
 */
     
#ifndef FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP 
#define FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP

#include <tuple>
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
    send(sz, dest, tag); // send msg size
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
    send(sz, dest, tag); // send msg size
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

  // api/impl for paracel default triple type isend
  MPI_Request isend(const paracel::triple_type & triple, int dest, int tag) {
    auto f = std::get<0>(triple);
    auto s = std::get<1>(triple);
    auto v = std::get<2>(triple);
    isend(f, dest, tag);
    isend(s, dest, tag);
    MPI_Request req = isend(v, dest, tag);
    return req;
  }

  // api/impl for std::tuple<size_t, size_t, double> type isend
  MPI_Request isend(const std::tuple<size_t, size_t, double> & triple, int dest, int tag) {
    auto f = std::get<0>(triple);
    auto s = std::get<1>(triple);
    auto v = std::get<2>(triple);
    isend(f, dest, tag);
    isend(s, dest, tag);
    MPI_Request req = isend(v, dest, tag);
    return req;
  }

  // api/impl for paracel list of default triple type isend
  MPI_Request isend(const paracel::list_type<paracel::triple_type> & triple_lst, int dest, int tag) {
    int sz = triple_lst.size(); // send container size
    send(sz, dest, tag);
    MPI_Request req;
    for(int i = 0; i < triple_lst.size(); ++i) {
      req = isend(triple_lst[i], dest, tag);
    }
    return req;
  }

  // api/impl for paracel list of triple<size_t, size_t, double> type isend
  MPI_Request isend(const paracel::list_type<std::tuple<size_t, size_t, double> > & triple_lst, int dest, int tag) {
    int sz = triple_lst.size(); // send container size
    send(sz, dest, tag);
    MPI_Request req;
    for(int i = 0; i < triple_lst.size(); ++i) {
      req = isend(triple_lst[i], dest, tag);
    }
    return req;
  }

  // api for paracel is_comm_builtin type recv
  // design tips: if return recv data, no template var in parameter
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    MPI_Status stat;
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Recv(&data, 1, dtype, src, tag, m_comm, &stat);
    return stat;
  }
  
  // api for paracel is_comm_container type recv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    int sz;
    recv(sz, src, tag);
    if(sz) data.resize(sz);
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

  // api/impl for paracel default triple type recv
  MPI_Status recv(paracel::triple_type & triple, int src, int tag) {
    recv(std::get<0>(triple), src, tag);
    recv(std::get<1>(triple), src, tag);
    MPI_Status stat = recv(std::get<2>(triple), src, tag);
    return stat;
  }
  
  // api/impl for std::tuple<size_t, size_t, double> type recv
  MPI_Status recv(std::tuple<size_t, size_t, double> & triple, int src, int tag) {
    recv(std::get<0>(triple), src, tag);
    recv(std::get<1>(triple), src, tag);
    MPI_Status stat = recv(std::get<2>(triple), src, tag);
    return stat;
  }
  
  // api/impl for list of paracel default triple type recv
  MPI_Status recv(paracel::list_type<paracel::triple_type> & triple_lst, int src, int tag) {
    int sz;
    recv(sz, src, tag);
    if(sz) triple_lst.resize(sz);
    MPI_Status stat;
    for(int i = 0; i < triple_lst.size(); ++i) {
      stat = recv(triple_lst[i], src, tag);
    }
    return stat;
  }
  
  // api/impl for list of std::tuple<size_t, size_t, double> type recv
  MPI_Status recv(paracel::list_type<std::tuple<size_t, size_t, double > > & triple_lst, int src, int tag) {
    MPI_Status stat;
    for(int i = 0; i = triple_lst.size(); ++i) {
      stat = recv(triple_lst[i], src, tag);
    }
    return stat;
  }

  void wait(MPI_Request & req) {
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  }

  // api for sendrecv
  template <class T>
  void sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
    MPI_Request req = isend(sdata, sto, stag);
    recv(rdata, rfrom, rtag);
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
  // impl with sendrecv because MPI_Alltoallv is really awful
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf) {
    if(sbuf.size()) rbuf.resize(sbuf.size());
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      auto tmpr = rbuf[0]; tmpr.clear();
      sendrecv(sbuf[t], tmpr, t, 2013, f, 2013);
      rbuf[t].insert(rbuf[t].end(), tmpr.begin(), tmpr.end());
    }
  }
  
  // api/impl for paracel default triple type alltoall
  void alltoall(const paracel::list_type<paracel::list_type<paracel::triple_type> > & sbuf, 
                paracel::list_type<paracel::list_type<paracel::triple_type> > & rbuf) {
    if(sbuf.size()) rbuf.resize(sbuf.size());
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      paracel::list_type<paracel::triple_type> tmpr;
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
