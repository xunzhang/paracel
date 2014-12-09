#include <time.h>
#include <math.h>
#include <iostream>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/QR>
#include <eigen3/Eigen/Cholesky>

int M = 4;
int N = 4;
int K = 3;

double loss(const Eigen::MatrixXd & A, const Eigen::MatrixXd & result) {
  return sqrt((A - result).squaredNorm() / (A.rows() * A.cols()));
}

Eigen::MatrixXd mf(const Eigen::MatrixXd & A,
                   Eigen::MatrixXd & W,
                   Eigen::MatrixXd & H) {
  for(int iter = 0; iter < 10; ++iter) {
    auto tmp1 = H.transpose() * H;
    W = A * H * tmp1.inverse();
    auto tmp2 = W.transpose() * W;
    H = A.transpose() * W * tmp2.inverse();
  }
  auto result = W * H.transpose();
  return result;
}

void qr_iteration(const Eigen::MatrixXd & M,
                  Eigen::MatrixXd & q,
                  Eigen::MatrixXd & r) {
  Eigen::MatrixXd MtM = M.transpose() * M;
  Eigen::LLT<Eigen::MatrixXd> lltOfA(MtM);
  Eigen::MatrixXd L = lltOfA.matrixL();
  r = -L.transpose();
  q = -M * L.transpose().inverse();
}

void subsvd(const Eigen::MatrixXd & r_W,
            const Eigen::MatrixXd & r_H,
            Eigen::MatrixXd & r_U) {
  std::cout << "r_W * r_H.transpose() " << r_W * r_H.transpose() << std::endl;
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(r_W * r_H.transpose(), Eigen::ComputeThinU | Eigen::ComputeThinV);
  r_U = svd.matrixU();
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
  std::cout << mtx << std::endl;
  //srand((unsigned)time(NULL));
  Eigen::MatrixXd H = Eigen::MatrixXd::Random(mtx.cols(), K);
  std::cout << "init H" << H << std::endl;
  Eigen::MatrixXd W(mtx.rows(), K);
  auto result = mf(mtx, W, H);
  std::cout << loss(mtx, result) << std::endl;
  std::cout << "W: " << W << std::endl;
  std::cout << "H: " << H << std::endl;
  std::cout << "approximate: " << W * H.transpose() << std::endl;
  Eigen::MatrixXd q_W, q_H, r_W, r_H;
  qr_iteration(W, q_W, r_W);
  qr_iteration(H, q_H, r_H);
  std::cout << q_W * r_W * r_H.transpose() * q_H.transpose() << std::endl;
  Eigen::MatrixXd r_U;
  subsvd(r_W, r_H, r_U);
  std::cout << "U: " << q_W * r_U << std::endl;
  return 0;
}
