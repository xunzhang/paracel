// mpic++ -std=c++11 -I../include test_scheduler.cpp ../src/libcomm.so ../src/libscheduler.so  /usr/lib/libboost_context.so
#include <iostream>
#include "paracel_types.hpp"
#include "load.hpp"
#include "utils.hpp"

int main(int argc, char *argv[]) {
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  int sz = comm.get_size();
  
  paracel::scheduler scheduler_obj(comm);
  paracel::list_type<paracel::str_type> name_lst{"d.txt"};
  auto loads = paracel::files_partition(name_lst, 2);
  auto linelst = scheduler_obj.schedule_load(loads);
  if(rk == 1) {
    for(auto & line : linelst) {
      std::cout << "line: " << line << std::endl;
    }
  }

  auto f_parser = std::bind(paracel::parser_a, std::placeholders::_1);
  auto result = scheduler_obj.lines_organize(linelst, f_parser);
  if(rk == 0) {
    std::cout << "aaaaaaaaaaaaaaaaaaaaaa" << std::endl;
    for(auto & lst : result) {
      for(auto & triple : lst) {
        std::cout << "rank: " << rk << std::get<0>(triple) << " " << std::get<1>(triple) << " " << std::get<2>(triple) << std::endl;
      }
      std::cout << "xxxxxxxxxxxxxxxxxxxxxx" << std::endl;
    }
  }
  
  auto stf = scheduler_obj.exchange(result);

  if(rk == 1) { 
    for(auto & tpl : stf) {
      std::cout << std::get<0>(tpl) << " " << std::get<1>(tpl) << " " << std::get<2>(tpl) << std::endl;
    }
  }

  return 0;
}
