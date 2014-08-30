#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/QR>
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
  std::cout << "----------" << std::endl;
  typedef Eigen::Triplet<double> eigen_triple;
  std::vector<eigen_triple> tpls;
  tpls.push_back(eigen_triple(0, 0, 0.6));
  tpls.push_back(eigen_triple(0, 2, 0.7));
  tpls.push_back(eigen_triple(0, 4, 0.4));
  tpls.push_back(eigen_triple(1, 2, 0.6));
  tpls.push_back(eigen_triple(1, 3, 0.5));
  tpls.push_back(eigen_triple(1, 4, 0.3));
  tpls.push_back(eigen_triple(2, 0, 0.3));
  tpls.push_back(eigen_triple(2, 1, 0.1));
  tpls.push_back(eigen_triple(3, 3, 0.1));
  tpls.push_back(eigen_triple(3, 4, 0.7));
  tpls.push_back(eigen_triple(4, 1, 0.3));
  tpls.push_back(eigen_triple(5, 0, 0.1));
  tpls.push_back(eigen_triple(5, 4, 0.7));
  tpls.push_back(eigen_triple(6, 0, 0.2));
  tpls.push_back(eigen_triple(6, 2, 0.8));
  tpls.push_back(eigen_triple(7, 0, 0.3));
  tpls.push_back(eigen_triple(8, 1, 0.1));
  tpls.push_back(eigen_triple(8, 2, 0.2));
  tpls.push_back(eigen_triple(8, 3, 0.3));
  tpls.push_back(eigen_triple(8, 4, 0.4));
  tpls.push_back(eigen_triple(9, 0, 0.9));
  tpls.push_back(eigen_triple(9, 3, 0.1));
  tpls.push_back(eigen_triple(9, 4, 0.2));
  Eigen::SparseMatrix<double, Eigen::RowMajor> A;
  A.resize(10, 5);
  A.setFromTriplets(tpls.begin(), tpls.end());
  Eigen::MatrixXd H(5, 3); // 5 * 3
  H << 1., 2., 3.,
      4., 5., 6.,
      7., 8., 9.,
      10., 11., 12.,
      13., 14., 15.;
  Eigen::MatrixXd W(1,1);
  W.resize(10, 3);
  W = A * H;
  std::cout << W << std::endl;
  Eigen::HouseholderQR<Eigen::MatrixXd> qr(W.transpose() * W);
  //qr.compute(W);
  Eigen::MatrixXd squareW(2,2);
  squareW << 1., 2., 3., 4.;
  std::cout << squareW.inverse() << std::endl;
  Eigen::MatrixXd R = qr.matrixQR().triangularView<Eigen::Upper>();
  std::cout << R << std::endl;
  std::cout << R.inverse() << std::endl;
  return 0;
}
