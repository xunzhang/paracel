#include <mpi.h>
#include <string>
#include "ps.hpp"
#include "utils/comm.hpp"

int main(int argc, char *argv[]) 
{
  //paracel::main_env comm_main_env(argc, argv);
  //paracel::Comm comm(MPI_COMM_WORLD);
  std::string s = "beater7:1,2,3,4PARACELbeater8:5,6,7,8";
  paracel::ps::parasrv obj(s);
  return 0;
}
