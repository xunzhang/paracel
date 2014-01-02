/**
 * unit test for comm.hpp(sz = 2)
 *
 */
#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <set>
#include <unordered_map>
#include <mpi.h>

#include "utils/comm.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD); 
  int rk = comm.get_rank();
  int sz = comm.get_size();

  { // alltoall
    std::vector<std::vector<std::tuple<std::string, std::string, double> > > a(4), b;
    if(rk == 0) {
      auto tpl1 = std::make_tuple("a","b",0.1);
      auto tpl2 = std::make_tuple("a","c",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v1{tpl1, tpl2};
      auto tpl3 = std::make_tuple("a","d",0.4);
      std::vector<std::tuple<std::string, std::string, double> > v2{tpl3};
      std::vector<std::tuple<std::string, std::string, double> > v3;
      auto tpl4 = std::make_tuple("b","a",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v4{tpl4};
      a[0] = v1;
      a[1] = v2;
      a[2] = v3;
      a[3] = v4;
    }   
    if(rk == 1) {
      std::vector<std::tuple<std::string, std::string, double> > v1;
      auto tpl3 = std::make_tuple("b","d",0.9);
      std::vector<std::tuple<std::string, std::string, double> > v2{tpl3};
      std::vector<std::tuple<std::string, std::string, double> > v3;
      auto tpl4 = std::make_tuple("e","d",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v4{tpl4};
      a[0] = v1;
      a[1] = v2;
      a[2] = v3;
      a[3] = v4;
    }
    
    if(rk == 2) {
      std::vector<std::tuple<std::string, std::string, double> > v1;
      auto tpl3 = std::make_tuple("e","d",0.3);
      std::vector<std::tuple<std::string, std::string, double> > v2{tpl3};
      auto tpl4 = std::make_tuple("d","c",0.4);
      std::vector<std::tuple<std::string, std::string, double> > v3{tpl4};
      auto tpl5 = std::make_tuple("k","q",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v4{tpl5};
      a[0] = v1;
      a[1] = v2;
      a[2] = v3;
      a[3] = v4;
    }
    if(rk == 3) {
      auto tpl1 = std::make_tuple("p","b",0.1);
      auto tpl2 = std::make_tuple("q","c",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v1{tpl1, tpl2};
      auto tpl3 = std::make_tuple("s","d",0.4);
      std::vector<std::tuple<std::string, std::string, double> > v2{tpl3};
      std::vector<std::tuple<std::string, std::string, double> > v3;
      auto tpl4 = std::make_tuple("r","a",0.2);
      std::vector<std::tuple<std::string, std::string, double> > v4{tpl4};
      a[0] = v1;
      a[1] = v2;
      a[2] = v3;
      a[3] = v4;
    }
    
    comm.alltoall(a, b); 
  }

  return 0;
}
