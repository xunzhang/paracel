#include <math.h>
#include <iostream>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>

int M = 4;
int N = 4;
int K = 3;

double loss(const Eigen::MatrixXd & A, const Eigen::MatrixXd & result) {
  return sqrt((A - result).squaredNorm() / (A.rows() * A.cols()));
}

Eigen::MatrixXd mf(const Eigen::MatrixXd & A) {
  Eigen::MatrixXd stone = Eigen::MatrixXd::Random(2, 3);
  Eigen::MatrixXd H(A.cols(), K);
  H.row(0) = stone.row(0);
  H.row(1) = stone.row(1);
  H.row(2) = stone.row(0);
  H.row(3) = stone.row(1);
  std::cout << "H " << H << std::endl;
  std::cout.precision(15);
  Eigen::MatrixXd W(A.rows(), K);
  for(int iter = 0; iter < 1; ++iter) {
    auto tmp1 = H.transpose() * H;
    std::cout << "HtH " << tmp1 << std::endl;
    W = A * H * tmp1.inverse();
    std::cout << "AH " << A * H << std::endl;
    std::cout << "HtH inverse " << tmp1.inverse() << std::endl;
    std::cout << "W " << W << std::endl;
    auto tmp2 = W.transpose() * W;
    std::cout << "debug2.5" << tmp2 << std::endl;
    H = A.transpose() * W * tmp2.inverse();
    std::cout << "debug3.0" << A.transpose() * W << std::endl;
    std::cout << "debug3.5" << H << std::endl;
  }
  auto result = W * H.transpose();
  std::cout << result << std::endl;
  //std::cout << result.(0, 0, 5, 5) << std::endl;
  return result;
}

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx(M, N);// = Eigen::MatrixXd::Random(M, N);
  //std::cout << mtx.block(0, 0, 5, 5) << std::endl;
  //std::cout << "---" << std::endl;
  mtx << 0, 0.583383, 0.585018, 0,
      0.585018, 0.161165, 0, 0.150756,
      0.583383, 0, 0.161165, 0.400892,
      0, 0.400892, 0.150756, 0;
  auto result = mf(mtx);
  std::cout << loss(mtx, result) << std::endl;
  
  return 0;
}
