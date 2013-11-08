/**
 * unit test for comm.hpp(sz = 2)
 *
 */
#include <iostream>
#include <vector>
#include <mpi.h>
#include "comm.hpp"
#include "paracel_types.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD); 
  int rk = comm.get_rank();
  int sz = comm.get_size();

  { // builtin send + recv
    if(rk == 0) {
      int a = 7; 
      comm.send(a, 1, 2013);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2013);
      std::cout << b << std::endl;
    }
  }

  { // isend + recv
    if(rk == 0) {
      int a = 7; 
      MPI_Request req;
      req = comm.isend(a, 1, 2013);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2013);
      std::cout << b << std::endl;
    }
  }

  { // container send + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      comm.send(aa, 1, 2013);
    } else if(rk == 1) {
      std::vector<int> bb(2);
      //bb.resize(2);
      comm.recv(bb, 0, 2013);
      for(auto & item : bb)
        std::cout << item << std::endl;
    }
  }

  { // container isend + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      MPI_Request req;
      req = comm.isend(aa, 1, 2013);
    } else if(rk == 1) {
      std::vector<int> bb(2);
      //bb.resize(2);
      comm.recv(bb, 0, 2013);
      for(auto & item : bb)
        std::cout << item << std::endl;
    }
  }

  { // builtin sendrecv
    int a = 8;
    int b;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(a, b, left, 2013, right, 2013);
  }

  { // container sendrecv
    std::vector<int> aaa{1,2,3};
    std::vector<int> bbb;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aaa, bbb, left, 2013, right, 2013);
    for(auto & item : bbb)
      std::cout << item << std::endl;
  }

  { // builtin bcast
    int a;
    if(rk == 0) a = 7;
    comm.bcast(a, 0);
    std::cout << "rk " << rk << " a " << a << std::endl;
  }

  { // container bcast
    std::vector<int> aa(2);
    if(rk == 0) {
      aa[0] = 3;
      aa[1] = 4;
    }
    comm.bcast(aa, 0);
    std::cout << "rk " << rk << " aa0 " << aa[0] << " aa1 " << aa[1] << std::endl;
  }
  
  { // builtin alltoall
    std::vector<int> a(2), b(2);
    if(rk == 0) {
      a[0] = 1;
      a[1] = 3;
    }
    if(rk == 1) {
      a[0] = 2;
      a[1] = 4;
    }
    comm.alltoall(a, b);
    std::cout << " rk " << rk << b[0] << " " << b[1] << std::endl;
  }

  { // container alltoall
    std::vector< std::vector<int> > a(2), b;
    if(rk == 0) {
      std::vector<int> tmp1{1, 5};
      std::vector<int> tmp2{3};
      a[0] = tmp1;
      a[1] = tmp2;
    }
    if(rk == 1) {
      std::vector<int> tmp1{2};
      std::vector<int> tmp2{4, 7};
      a[0] = tmp1;
      a[1] = tmp2;
    }
    comm.alltoall(a, b);
    if(rk == 1) {
      for(auto & item : b[0])
        std::cout << " test " << item << " ";
      std::cout << "sep" << std::endl;
      for (auto & item : b[1])
        std::cout << " test " << item << " ";
      std::cout << std::endl;
    }
  }

  { // builtin allreduce
    int aaa;
    if(rk == 0) { aaa = 1; }
    if(rk == 1) { aaa = 2; }
    auto f = [](){ int b = 1; };
    comm.allreduce(aaa, f);
    std::cout << " rk " << rk << " result " << aaa << std::endl;
  }
  
  { // container allreduce
    std::vector<int> aaa(3);
    if(rk == 0) {
      aaa[0] = 1;
      aaa[1] = 2;
      aaa[2] = 3;
    }
    if(rk == 1) {
      aaa[0] = 3;
      aaa[1] = 2;
      aaa[2] = 1;
    }
    auto f = [](){ int b = 1; };
    comm.allreduce(aaa, f);
    for(auto & item : aaa)
      std::cout << " rk " << rk << " result aaa " << item << std::endl;
  }

  return 0;
}
