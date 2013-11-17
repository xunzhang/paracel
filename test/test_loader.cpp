#include <functional>
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "load/loader.hpp"
#include "load/parser.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  auto f_parser = std::bind(paracel::parser_a, std::placeholders::_1);
  paracel::loader<paracel::str_type> ld("c.txt", comm, f_parser, "fmap");
  paracel::dict_type<size_t, paracel::str_type> rm, cm;
  paracel::list_type<std::tuple<size_t, size_t, double> > stf_new;
  ld.load(rm, cm);
  for(auto & item : rm) {
    std::cout << rk << " "<< item.first << " ~ " << item.second << std::endl;
  }
  for(auto & item : cm) {
    std::cout << rk << " " << item.first << " ^ " << item.second << std::endl;
  } 
  return 0;
}
