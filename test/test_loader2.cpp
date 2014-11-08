#include <string>
#include <functional>
#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "load/loader.hpp"
#include "load/parser.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();

  auto parser = [](const std::string & line) { return paracel::str_split(line, ','); }; 
  auto f_parser = paracel::gen_parser(parser);

  {
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/paracel/demo/d2.txt", comm, f_parser, "fmap");
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/paracel/demo/unit.txt", comm, f_parser, "fmap");
    paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/paracel/test/serial/training_test.csv", comm, f_parser, "fmap");
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/paracel/test/serial/training.csv", comm, f_parser, "fmap");
    auto linelst = ld.load();
    // graph test
    paracel::digraph<paracel::str_type> grp;
    ld.create_graph(linelst, grp);
    auto local_f = [&] (const paracel::str_type & a , 
    			const paracel::str_type & b, 
			double c) {
      if(rk == 1) {
        std::cout << a << " A " << b << " A " << c << std::endl;
      }
    };
    grp.traverse(local_f);
  }
  return 0;
}
