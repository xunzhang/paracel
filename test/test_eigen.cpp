#include <iostream>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include "utils.hpp"

int main(int argc, char *argv[])
{
  // Eigen::Matrix is static type which Eigen::MatrixXd is dynamic
  Eigen::Matrix<double, 3, 3, Eigen::RowMajor> m = Eigen::Matrix3d::Identity();
  std::cout << m.size() << std::endl;
  m.col(1) = Eigen::Vector3d(4, 5, 6);
  std::cout << m.IsRowMajor << std::endl;
  std::cout << "---" << std::endl;
  auto v = paracel::mat2vec(m);
  for(auto & vv : v) {
    std::cout << vv << std::endl;
  }
  std::cout << "---" << std::endl;
  Eigen::MatrixXd mat = paracel::vec2mat(v, 3);
  std::cout << mat << std::endl;
  std::cout << "---" << std::endl;
  Eigen::VectorXd vv = m.col(1);
  auto vvv = paracel::evec2vec(vv);
  for(auto & vv : vvv) {
    std::cout << vv << std::endl;
  }
  std::cout << "---" << std::endl;
  Eigen::VectorXd vvvv = paracel::vec2evec(vvv);
  std::cout << vvvv << std::endl;
  return 0;
}
