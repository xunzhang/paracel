// mpic++ -std=c++11 -I../include test_index_mapping.cpp /usr/lib/libboost_context.so ../src/comm.so ../src/scheduler.so
#include <iostream>
#include <tuple>
#include "paracel_types.hpp"
#include "load.hpp"

int main(int argc, char *argv[]) {
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  int sz = comm.get_size();

  paracel::scheduler scheduler_obj(comm);
  paracel::lt_type slotslst;
  if(rk == 0) {
    slotslst.push_back(std::make_tuple("64", "6", 3.));
    slotslst.push_back(std::make_tuple("64", "42", 4.));
  }
  if(rk == 1) {
    slotslst.push_back(std::make_tuple("64", "21", 1.));
  }
  if(rk == 2) {
    slotslst.push_back(std::make_tuple("27", "28", 1.));
    slotslst.push_back(std::make_tuple("27", "42", 2.));
    slotslst.push_back(std::make_tuple("37", "42", 3.));
    slotslst.push_back(std::make_tuple("29", "28", 1.));
    slotslst.push_back(std::make_tuple("29", "6", 3.));
  }
  if(rk == 3) {
    slotslst.push_back(std::make_tuple("37", "21", 5.));
  }
  paracel::dict_type<size_t, paracel::str_type> rm, cm;
  paracel::list_type<std::tuple<size_t, size_t, double> > stf;
  paracel::dict_type<size_t, int> dm;
  paracel::dict_type<size_t, int> col_dm;
  scheduler_obj.index_mapping(slotslst, stf, rm, cm, dm, col_dm);
  
  if(rk == 2) {
    for(auto & item : stf)
      std::cout << std::get<0>(item) << " | " << std::get<1>(item) << " | "<< std::get<2>(item) << std::endl;
    for(auto & item : dm)
      std::cout << item.first << " - " << item.second << std::endl;
    for(auto & item : col_dm)
      std::cout << item.first << " * " << item.second << std::endl;
  }

  return 0;
}
