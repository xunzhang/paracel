#include <vector>
#include <iostream>
#include <chrono>

#include "utils/comm.hpp"

#define N 30000000

int main(int argc, char *argv[])
{
  std::vector<double> data;
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  int sz = comm.get_size();
  
  if(rk == 0) {
    for(size_t i = 0; i < N; ++i) {
      data.push_back(3.14);
    }
  }

  {
    std::cout << "p2p isend/recv test" << std::endl;
    if(rk == 0) {
      comm.isend(data, 1, 2014);
    }
    if(rk == 1) {
      auto start = std::chrono::high_resolution_clock::now();
      comm.recv(data, 0, 2014);
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(end - start);
      std::cout << "p2p isend/recv test" << elapsed.count() << std::endl;
    }
  }
  return 0;
}
