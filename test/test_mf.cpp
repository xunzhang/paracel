#include <math.h>
#include <string>
#include <functional>
#include <eigen/Eigen/Sparse>
#include <eigen/Eigen/Dense>
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "load/loader.hpp"
#include "load/parser.hpp"

void mf(Eigen::SparseMatrix<double, Eigen::RowMajor> & A, 
  	Eigen::MatrixXd & W, 
    	Eigen::MatrixXd & H) {

  for(int it = 0; it < 2; ++it) {
    W = A * H * (H.transpose() * H).inverse();
    H = ((W.transpose() * W).inverse() * W.transpose() * A).transpose();
    std::cout << "iter done" << std::endl;
  }
}

double rmse(Eigen::SparseMatrix<double, Eigen::RowMajor> & A,
	    Eigen::MatrixXd & W,
            Eigen::MatrixXd & H) {
  double rmse = 0.;
  //Eigen::MatrixXd R = W * H.transpose();
  int cnt = 0;
  for (int i = 0; i < A.outerSize(); ++i) {
    for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(A, i); it; ++it) {
      int ii = it.row();
      int jj = it.col();
      double tmp = W.row(ii) * H.transpose().col(jj);
      //if(tmp < 0) { tmp = 3.85115466306; }
      std::cout << "compare: " << tmp << " original" << it.value() << std::endl;
      rmse += (tmp - it.value()) * (tmp - it.value());
      cnt += 1;
      //std::cout << it.row() << " | " << it.col() << " | " << it.value() << std::endl;
      //rmse += (R(it.row(), it.col()) - it.value()) * (R(it.row(), it.col()) - it.value());
    }
  }
  return sqrt(rmse / (double)cnt);
}

double rmse_test(paracel::list_type<paracel::str_type> & linelst,
	    	Eigen::MatrixXd & W,
            	Eigen::MatrixXd & H,
		paracel::dict_type<paracel::str_type, size_t> rmp, 
		paracel::dict_type<paracel::str_type, size_t> cmp) {
  double rmse = 0.;
  int cnt = 0;
  for(auto & line : linelst) {
    auto tpl = paracel::str_split(line, ',');
    int ii = rmp[tpl[0]];
    int jj = cmp[tpl[1]];
    double tmp = W.row(ii) * H.transpose().col(jj);
    auto temp = tmp - std::stod(tpl[2]);
    rmse += temp * temp;
    cnt += 1;
  }
  return sqrt(rmse / (double)cnt);
}

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();

  auto f_parser = std::bind(paracel::parser_a, std::placeholders::_1, ',');
  auto f2_parser = std::bind(paracel::parser_a, std::placeholders::_1, ' ');

  {
    //paracel::loader<paracel::str_type> ld("./mtx.csv", comm, f_parser, "fmap"); 							// fsmap, smap
    //paracel::loader<paracel::str_type> ld("/home/xunzhang/xunzhang/Data/test/netflix_small/train/train2", comm, f_parser, "fmap"); 		// fsmap, smap
    paracel::loader<paracel::str_type> ld("/home/xunzhang/xunzhang/Proj/libmf-1.0/smalldata", comm, f2_parser, "linesplit"); 		// fsmap, smap
    auto linelst = ld.load();
    paracel::dict_type<size_t, paracel::str_type> rm, cm;
    Eigen::SparseMatrix<double, Eigen::RowMajor> blk_mtx;
    ld.create_matrix(linelst, blk_mtx, rm, cm);
    
    paracel::dict_type<paracel::str_type, size_t> rrm, rcm;
    for(auto & kv : rm) {
      rrm[kv.second] = kv.first;
    }
    for(auto & kv : cm) {
      rcm[kv.second] = kv.first;
    }

    int m = blk_mtx.rows();
    int n = blk_mtx.cols();
    std::cout << m << " " << n << std::endl;
    int k = 40;
    Eigen::MatrixXd W = Eigen::MatrixXd(m, k); //::Random(m, k);
    Eigen::MatrixXd H = Eigen::MatrixXd::Random(n, k);

    for(int i = 0; i < m; ++i) {
      for(int j = 0; j < k; ++j) {
        H.coeffRef(i,j) = (H.coeffRef(i, j) + 1) * 0.5;
      }
    }
    
    mf(blk_mtx, W, H);

    std::cout << "train: " << rmse(blk_mtx, W, H) << std::endl;

    //paracel::loader<paracel::str_type> ld2("./mtx2.csv", comm, f_parser, "fmap"); 							// fsmap, smap
    //paracel::loader<paracel::str_type> ld2("/home/xunzhang/xunzhang/Data/test/netflix_small/test/test2", comm, f_parser, "fmap"); 		// fsmap, smap
    paracel::loader<paracel::str_type> ld2("/home/xunzhang/xunzhang/Data/test/netflix_small/test/test2", comm, f_parser, "fmap"); 		// fsmap, smap
    auto linelst2 = ld2.load();
    std::cout << "predict: " << rmse_test(linelst2, W, H, rrm, rcm) << std::endl;
  }
  return 0;
}
