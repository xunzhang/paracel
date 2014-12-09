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

  auto f_parser = std::bind(paracel::parser_a, std::placeholders::_1);
  auto f1_parser = std::bind(paracel::parser_a, std::placeholders::_1, ',');
  auto f2_parser = std::bind(paracel::parser_b, std::placeholders::_1, ' ', '|');
  auto f3_parser = [](const std::string & line) { return paracel::str_split(line, ' '); }; 
  auto f4_parser = paracel::gen_parser(paracel::parser_a);
  auto f5_parser = paracel::gen_parser(paracel::parser_b, ' ', '|');

  {
    //paracel::loader<paracel::str_type> ld("../demo/a.txt", comm, f_parser, "fmap", true); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/a2.txt", comm, f2_parser, "fmap", true); 	// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/b.txt", comm, f4_parser, "fmap"); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/d2.txt", comm, f_parser, "fmap"); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/c.txt", comm, f3_parser, "fmap", true); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/f.txt", comm, f5_parser, "fmap", true); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("../demo/f.txt", comm, f5_parser, "fmap", true); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/Data/training1.csv", comm, f1_parser, "linesplit"); 		// fsmap, smap
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/tmp/movielen100k", comm, f1_parser, "linesplit"); 		// fsmap, smap
    paracel::loader<paracel::str_type> ld("../demo/unit.txt", comm, f1_parser, "fmap");
    auto linelst = ld.load();
    paracel::dict_type<size_t, paracel::str_type> rm, cm;
    paracel::dict_type<size_t, int> dm, col_dm;
    Eigen::SparseMatrix<double, Eigen::RowMajor> blk_mtx;
    ld.create_matrix(linelst, blk_mtx, rm, cm, dm, col_dm);
    if(rk == 0) {
      std::cout << blk_mtx << std::endl;
    
      for(auto & item : dm) {
        std::cout << rk << " "<< item.first << " ~ " << item.second << std::endl;
      }
      for(auto & item : col_dm) {
        std::cout << rk << " " << item.first << " ^ " << item.second << std::endl;
      }
    }

    // graph test
    paracel::digraph<paracel::str_type> grp;
    ld.create_graph(linelst, grp);
    auto local_f = [&] (const paracel::str_type & a , 
    			const paracel::str_type & b, 
			double c) {
      if(rk == 0) {
        std::cout << a << " A " << b << " A " << c << std::endl;
      }
    };
    grp.traverse(local_f);
    /*
    if(rk == 1) {
      std::cout << blk_mtx << std::endl;
    }
    */
  }
  /*
  {
    paracel::loader<paracel::str_type> ld("/home/xunzhang/xunzhang/Proj/paracel/demo/g.txt", comm, f_parser, "linesplit");
    auto linelst = ld.load();
    Eigen::MatrixXd d_blk_mtx;
    paracel::dict_type<size_t, paracel::str_type> rm;
    ld.create_matrix(linelst, d_blk_mtx, rm);
    if(rk == 0) {
      std::cout << d_blk_mtx << std::endl;
    }
  }
  {
    paracel::loader<paracel::str_type> ld("/home/xunzhang/xunzhang/Proj/paracel/demo/f.txt", comm, f5_parser, "fsmap", true); 		// fsmap, smap
    auto linelst = ld.load();
    paracel::dict_type<size_t, paracel::str_type> rm, cm;
    Eigen::SparseMatrix<double, Eigen::RowMajor> blk_mtx;
    paracel::dict_type<size_t, int> dm, col_dm;
    ld.create_matrix(linelst, blk_mtx, rm, cm, dm, col_dm);
    for(auto & item : rm) {
      std::cout << rk << " "<< item.first << " ~ " << item.second << std::endl;
    }
    for(auto & item : cm) {
      std::cout << rk << " " << item.first << " ^ " << item.second << std::endl;
    }
    if(rk == 3)
      std::cout << "last" << blk_mtx << std::endl;
  }
  */
  return 0;
}
