#include <math.h>
#include <iostream>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/QR>

int M = 100;
int N = 80;
int K = 10;

double loss(const Eigen::MatrixXd & A, const Eigen::MatrixXd & result) {
  return sqrt((A - result).squaredNorm() / (A.rows() * A.cols()));
}

Eigen::MatrixXd random_projection(const Eigen::MatrixXd & A) {
  Eigen::MatrixXd H = Eigen::MatrixXd::Random(A.cols(), K);
  Eigen::MatrixXd W(A.rows(), K);
  for(int iter = 0; iter < 10; ++iter) {
    W = A * H;
    H = A.transpose() * W;
    Eigen::HouseholderQR<Eigen::MatrixXd> qr(H.transpose() * H);
    Eigen::MatrixXd R = qr.matrixQR().triangularView<Eigen::Upper>();
    H = H * R.inverse();
  }
  W = A * H;
  //std::cout << W << std::endl;
  //std::cout << "---" << std::endl;
  //std::cout << H << std::endl;
  //std::cout << "---" << std::endl;
  auto result = W * H.transpose();
  //std::cout << result << std::endl;
  std::cout << result.block(0, 0, 5, 5) << std::endl;
  return result;
}

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx = Eigen::MatrixXd::Random(M, N);
  //std::cout << mtx << std::endl;
  std::cout << mtx.block(0, 0, 5, 5) << std::endl;
  std::cout << "---" << std::endl;
  auto result = random_projection(mtx);
  std::cout << loss(mtx, result) << std::endl;

  std::cout << "~~~" << std::endl;
  Eigen::MatrixXd mmtx(3, 3);
  mmtx << 1., 2., 3., 4., 5., 6., 7., 8., 9;
  std::cout << sqrt(mmtx.squaredNorm() / 9) << std::endl;
  return 0;
}
