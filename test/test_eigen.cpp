#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
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
  
  std::cout << "---" << std::endl;
  
  Eigen::Vector3d vec(1,2,3);
  auto r = m.row(2) * vec;
  std::cout << r << std::endl;
  
  std::cout << "---" << std::endl;
  
  Eigen::Matrix3d mm = Eigen::Matrix3d::Random();
  std::cout << "Here is the matrix m:" << std::endl << mm << std::endl;
  std::cout << "Here is the sum of each column:" << std::endl << mm.colwise().sum() << std::endl;
  std::cout << "Here is the sum of each row:" << std::endl << mm.rowwise().sum() << std::endl;
  std::cout << "Here is the maximum absolute value of each column:"
           << std::endl << mm.cwiseAbs().colwise().maxCoeff() << std::endl;

  std::cout << "---" << std::endl;

  Eigen::MatrixXd mtx(3, 2);
  Eigen::VectorXd vech(2);
  vech[0] = 1.1; vech[1] = 0.8;
  mtx.row(0) = vech;
  vech[0] = 2.; vech[1] = 2.;
  mtx.row(1) = vech;
  vech[0] = 1.; vech[1] = 0.5;
  mtx.row(2) = vech;
  vech[0] = 1.01; vech[1] = 0.28;
  Eigen::MatrixXd::Index indx;
  std::cout << mtx << std::endl;
  (mtx.rowwise() - vech.transpose()).rowwise().squaredNorm().minCoeff(&indx);
  std::cout << indx << std::endl;
  std::unordered_map<std::string, int> mmap;
  mmap["sda"] = indx;

  std::cout << "---" << std::endl;
  
  std::cout << mtx << std::endl;
  std::cout << vech << std::endl;
  std::cout << mtx * vech << std::endl;
  mtx.row(0) *= 10;
  std::cout << mtx << std::endl;

  std::cout << "---" << std::endl;
  Eigen::Matrix2d mmat;
  mmat << 1, 2,
          3, 4;
  Eigen::VectorXd vvec(2);
  int rr = -1, cc = -1;
  vvec << 1.1, 2.2;
  std::cout << mmat.maxCoeff(&rr, &cc) << std::endl;
  std::cout << rr << " | " << cc << std::endl;
  return 0;
}
