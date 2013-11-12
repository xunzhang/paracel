// mpic++ -std=c++11 -I../include test_scheduler.cpp ../src/libcomm.so ../src/libscheduler.so  /usr/lib/libboost_context.so
#include <iostream>
#include "paracel_types.hpp"
#include "load.hpp"
#include "utils.hpp"
//#include "load/scheduler.hpp"
//#include "utils/comm.hpp"
//#include "load/partition.hpp"

int main(int argc, char *argv[]) {
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  int sz = comm.get_size();
  
  paracel::scheduler scheduler_obj(comm);
  paracel::list_type<paracel::str_type> name_lst{"a.txt", "b.txt"};
  auto loads = paracel::files_partition(name_lst, 4);
  auto linelst = scheduler_obj.schedule_load(loads);
  if(rk == 1) {
    for(auto & line : linelst) {
      std::cout << "line: " << line << std::endl;
    }
  }
  
  return 0;
}
