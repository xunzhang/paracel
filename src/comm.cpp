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
 *   Hong Wu <xunzhangthu@gmail.com>
 *   Changsheng Jiang <jiangzuoyan@gmail.com>
 *
 */

/**
 * a simple version, just for paracel usage
 * full version is implemented at Douban by Changsheng Jiang
 *
 */
#include <iostream>
#include "comm.hpp"
#include "paracel_types.hpp"

namespace paracel {

Comm::Comm(MPI_Comm comm) {
  init(comm);
}

Comm::Comm(const Comm &r) {
  init(r.m_comm);
}

Comm::Comm(Comm &&r) {
  m_comm = r.m_comm;
  r.m_comm = MPI_COMM_NULL;
  m_rk = r.m_rk;
  m_sz = r.m_sz;
}

Comm::~Comm() {
  MPI_Comm_free(&m_comm);
}

Comm& Comm::operator=(const Comm &r) {
  MPI_Comm_free(&m_comm);
  init(r.m_comm);
  return *this;
}

Comm& Comm::operator=(Comm &&r) {
  MPI_Comm_free(&m_comm);
  m_comm = r.m_comm;
  r.m_comm = MPI_COMM_NULL;
  m_rk = r.m_rk;
  m_sz = r.m_sz;
  return *this;
}

void Comm::init(MPI_Comm comm) {
  MPI_Comm_dup(comm, &m_comm);
  MPI_Comm_rank(m_comm, &m_rk);
  MPI_Comm_size(m_comm, &m_sz);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<T>::value>
Comm::send(const T &data, int dest, int tag) {
  MPI_Datatype dtype = paracel::datatype<T>();
  MPI_Send((void *)&data, 1, dtype, dest, tag, m_comm);
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::send(const T &data, int dest, int tag) {
  int sz = (int)data.size();
  send(data, sz, dest, tag);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
Comm::send(const T &data, int sz, int dest, int tag) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  MPI_Send((void *)&data[0], sz, dtype, dest, tag, m_comm);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Request> 
Comm::isend(const T &data, int dest, int tag) {
  MPI_Request req;
  MPI_Datatype dtype = paracel::datatype<T>();
  MPI_Isend((void *)&data, 1, dtype, dest, tag, m_comm, &req);
  return req;
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Request>
Comm::isend(const T & data, int dest, int tag) {
  int sz = (int)data.size();
  return isend(data, sz, dest, tag);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, MPI_Request>
Comm::isend(const T & data, int sz, int dest, int tag) {
  MPI_Request req;
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  MPI_Isend((void *)&data[0], sz, dtype, dest, tag, m_comm, &req);
  return req;
}

// design tip:
// if return recv data, no template parameter in parameter
template <class T>
paracel::Enable_if<paracel::is_comm_builtin<T>::value>
Comm::recv(T & data, int src, int tag) {
  MPI_Status stat;
  MPI_Datatype dtype = paracel::datatype<T>();
  MPI_Recv(&data, 1, dtype, src, tag, m_comm, &stat);
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::recv(T & data, int src, int tag) {
  int sz = (int)data.size();
  recv(data, sz, src, tag);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
Comm::recv(T & data, int sz, int src, int tag) {
  MPI_Status stat;
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  MPI_Recv((void *)&data[0], sz, dtype, src, tag, m_comm, &stat);
}

void Comm::wait(MPI_Request & req) {
  MPI_Wait(&req, MPI_STATUS_IGNORE);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<T>::value>
Comm::sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
  //MPI_Status stat;
  //MPI_Datatype dtype = paracel::datatype<T>();
  //MPI_Sendrecv((void *)&sdata, 1, dtype, sto, stag, (void *)&rdata, 1, dtype, rfrom, rtag, m_comm, &stat);
  MPI_Request req = isend(sdata, sto, stag);
  recv(rdata, rfrom, rtag);
  wait(req);
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
  int sz = (int)sdata.size();
  rdata.resize(sz);
  sendrecv(sdata, sz, rdata, sto, stag, rfrom, rtag);
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::sendrecv(const T &sdata, int sz, T & rdata, int sto, int stag, int rfrom, int rtag) {
  MPI_Request req = isend(sdata, sz, sto, stag);
  recv(rdata, sz, rfrom, rtag);
  wait(req);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<T>::value>
Comm::bcast(T & data, int master) {
  MPI_Datatype dtype = paracel::datatype<T>();
  MPI_Bcast((void *)&data, 1, dtype, master, m_comm);
}

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::bcast(T & data, int master) {
  int sz = (int)data.size();
  bcast(data, sz, master);
}

template <class T>
paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
Comm::bcast(T & data, int sz, int master) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  MPI_Bcast((void *)&data[0], sz, dtype, master, m_comm);
}

template <class T, class F>
void Comm::bcastring(const T & data, F & func) {
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

template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_builtin<typename T::value_type>::value>
Comm::alltoall(const T & sbuf, T & rbuf) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  MPI_Alltoall((void *)&sbuf[0], 1, dtype, (void *)&rbuf[0], 1, dtype, m_comm); 
}

// MPI_Alltoallv is really awful
template <class T>
paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
Comm::alltoall(const T & sbuf, T & rbuf) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
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

template <class T, class F>
paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
Comm::alltoallring(const T & sbuf, T & rbuf, F & func) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
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

// TODO: abstract MPI_SUM with func
template <class T, class F>
paracel::Enable_if<paracel::is_comm_builtin<T>::value>
Comm::allreduce(T & data, F & func) {
  MPI_Datatype dtype = paracel::datatype<T>();
  T tmp;
  MPI_Allreduce((void *)&data, (void *)&tmp, 1, dtype, MPI_SUM, m_comm);
  data = tmp;
}

// TODO: abstract MPI_SUM with func
template <class T, class F>
paracel::Enable_if<paracel::is_comm_container<T>::value>
Comm::allreduce(T & data, F & func) {
  int sz = (int)data.size();
  allreduce(data, sz, func);
}

template <class T, class F>
paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
Comm::allreduce(T & data, int sz, F & func) {
  MPI_Datatype dtype = paracel::container_inner_datatype<T>();
  T tmp;
  tmp.resize(data.size());
  MPI_Allreduce((void *)&data[0], (void *)&tmp[0], sz, dtype, MPI_SUM, m_comm);
  for(int i = 0; i < data.size(); ++i) {
    data[i] = tmp[i];
  }
}

} // namespace paracel
