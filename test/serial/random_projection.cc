#include <iostream>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/QR>

int K = 100;
int N = 1000;

void random_projection(const Eigen::MatrixXd & A) {
  Eigen::MatrixXd H = Eigen::MatrixXd::Random(N, K);
  Eigen::MatrixXd W(N, K);
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
  std::cout << result.block(0, 0, 5, 5) << std::endl;
}

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx = Eigen::MatrixXd::Random(N, N);
  std::cout << mtx.block(0, 0, 5, 5) << std::endl;
  std::cout << "---" << std::endl;
  random_projection(mtx);
  return 0;
}
