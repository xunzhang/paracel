#include <iostream>
#include <tuple>
#include <string>

#include "utils.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();
  int sz = comm.get_size();

  std::vector<std::vector<std::tuple<std::string, std::string, double> > > a(2), b;
  if(rk == 0) {
    std::tuple<std::string, std::string, double> tpl1("a", "b", 1.);
    std::tuple<std::string, std::string, double> tpl2("a", "c", 1.);
    std::vector<std::tuple<std::string, std::string, double> > tmp = {tpl1, tpl2};
    a[1] = tmp;
  }
  if(rk == 1) {
    std::tuple<std::string, std::string, double> tpl1("b", "a", 1.);
    std::tuple<std::string, std::string, double> tpl2("c", "b", 1.);
    std::vector<std::tuple<std::string, std::string, double> > tmp = {tpl1, tpl2};
    a[0] = tmp;
  }
  comm.alltoall(a, b);
  if(rk == 1) {
    for(auto & ii : b[0]) {
      std::cout << std::get<0>(ii) << "-" << std::get<1>(ii) << "-" << std::get<2>(ii) << std::endl;
    }
  }
  return 0;
}
