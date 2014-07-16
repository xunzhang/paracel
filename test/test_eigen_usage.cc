#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include "utils.hpp"

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx(4, 3);
  std::cout << mtx << std::endl;
  mtx << 1., 2., 3.,
        4., 5., 6.,
        7., 8., 9.,
        10., 11., 12.;
  Eigen::MatrixXd clusters_mtx(2, 3);
  std::vector<double> c1 = {2.5, 3.5, 4.5}, c2 = {8.5, 9.5, 10.5};
  clusters_mtx.row(0) = paracel::vec2evec(c1);
  clusters_mtx.row(1) = paracel::vec2evec(c2);

  for(size_t i = 0; i < (size_t)mtx.rows(); ++i) {
    Eigen::MatrixXd::Index indx;
    (clusters_mtx.rowwise() - mtx.row(i)).rowwise().squaredNorm().minCoeff(&indx);
    std::cout << "| " << indx << " |" << std::endl;
  }

  return 0;
}
