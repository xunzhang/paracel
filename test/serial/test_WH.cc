#include <math.h>
#include <iostream>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>

int M = 1000;
int N = 800;
int K = 40;

double loss(const Eigen::MatrixXd & A, const Eigen::MatrixXd & result) {
  return sqrt((A - result).squaredNorm() / (A.rows() * A.cols()));
}

Eigen::MatrixXd mf(const Eigen::MatrixXd & A) {
  Eigen::MatrixXd H = Eigen::MatrixXd::Random(A.cols(), K);
  Eigen::MatrixXd W(A.rows(), K);
  for(int iter = 0; iter < 10; ++iter) {
    auto tmp1 = H.transpose() * H;
    W = A * H * tmp1.inverse();
    auto tmp2 = W.transpose() * W;
    H = A.transpose() * W * tmp2.inverse();
  }
  auto result = W * H.transpose();
  std::cout << result.block(0, 0, 5, 5) << std::endl;
  return result;
}

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx = Eigen::MatrixXd::Random(M, N);
  std::cout << mtx.block(0, 0, 5, 5) << std::endl;
  std::cout << "---" << std::endl;
  auto result = mf(mtx);
  std::cout << loss(mtx, result) << std::endl;
  return 0;
}
