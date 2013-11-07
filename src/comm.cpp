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
 *   Changsheng Jiang <jiangzuoyan@gmail.com>
 *   Hong Wu <xunzhangthu@gmail.com>
 *
 */
	   
#include "comm.hpp"

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

Comm& operator=(const Comm &r) {
  MPI_Comm_free(&m_comm);
  init(r.m_comm);
  return *this;
}

Comm& operator=(Comm &&r) {
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
void Comm::send(const T & data, int dest, int tag) {
}

template <class T>
MPI_Request& isend(const T & data, int dest, int tag) {
}

template <class T>
T& Comm::recv(int src, int tag, MPI_Status *status = MPI_STATUS_IGNORE) {
}

template <class T>
T& sendrecv(const T & data, int sto, int stag, int rfrom, int rtag) {

}

template <class T, class F>
void bcastring(const T & data, F & func) {

}

template <class T>
T& alltoall(const T&) {

}

} // namespace paracel
