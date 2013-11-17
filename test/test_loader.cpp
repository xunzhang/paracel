#include <functional>
#include <eigen/Eigen/Sparse>
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
  
  auto linelst = ld.load();
  
  paracel::dict_type<size_t, paracel::str_type> rm, cm;
  paracel::dict_type<size_t, int> dm, col_dm;
  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_mtx;
  ld.create_matrix(linelst, blk_mtx, rm, cm, dm, col_dm);
  
  for(auto & item : rm) {
    std::cout << rk << " "<< item.first << " ~ " << item.second << std::endl;
  }
  for(auto & item : cm) {
    std::cout << rk << " " << item.first << " ^ " << item.second << std::endl;
  } 
  if(rk == 0) {
    std::cout << rk << blk_mtx << std::endl;
  }
  return 0;
}
