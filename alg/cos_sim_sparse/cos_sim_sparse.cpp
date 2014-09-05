/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <google/gflags.h>
#include <eigen3/Eigen/Sparse>
#include "utils.hpp"
#include "ps.hpp"

using std::string;
using std::vector;
using std::unordered_map;

namespace paracel {

auto local_parser = [] (const string & line) {
  auto r = paracel::str_split(line, ',');
  return r;
};

class cos_sim_sparse : public paracel::paralg {
 
 public:
  cos_sim_sparse(paracel::Comm comm, 
                         string hosts_dct_str,
                         string _input, 
                         string _output) : 
      paracel::paralg(hosts_dct_str, comm, _output),
      input(_input),
      output(_output) {}

  virtual ~cos_sim_sparse() {}

  virtual void solve() {
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_matrix(blk_A_T, row_map, col_map, input, f_parser, "smap");
    
    blk_size = blk_A_T.rows() / 5000; // 0.05MB * n/p = 0.05 * 10000 MB = 500MB
    if(blk_size == 0) {
      blk_size = 1;
    }
    for(int k = 0; k < blk_size; ++k) {
      int cols = blk_A_T.cols();
      int rows = blk_A_T.rows() / blk_size;
      if(k == blk_size - 1) {
        rows += blk_A_T.rows() % blk_size;
      }
      Eigen::MatrixXd part_blk_A_T = Eigen::MatrixXd(blk_A_T).block(k * blk_size, 0, rows, cols);
      Eigen::MatrixXd local_result = part_blk_A_T * blk_A_T.transpose();
      if(get_worker_id() == 1) {
        std::cout << local_result << std::endl;
      }
      std::vector<double> vec_buff = paracel::mat2vec(local_result);
      paracel_bupdate("result_" + std::to_string(k), 
                      vec_buff, 
                      "/mfs/user/wuhong/paracel/local/lib/libcos_sim_sparse_update.so", 
                      "cos_sim_sparse_updater");
    }
    sync();
  }

  void dump() {
    if(get_worker_id() == 0) {
      // pull result
      for(int k = 0; k < blk_size; ++k) {
        int rows = blk_A_T.rows() / blk_size;
        if(k == blk_size - 1) {
          rows += blk_A_T.rows() % blk_size;
        }
        auto data = paracel_read<std::vector<double> >("result_" + std::to_string(k));
        auto blk_mtx_result = paracel::vec2mat(data, rows);
        std::cout << blk_mtx_result << std::endl;
        result.push_back(blk_mtx_result);
      }
      /*
      // dump
      std::unordered_map<std::string, std::vector<std::pair<std::string, double> > > r;
      for(size_t k = 0; k < result.size(); ++k) {
        for(size_t i = 0; i < (size_t)result[i].rows(); ++i) {
          for(size_t j = 0; j < (size_t)result[i].cols(); ++j) {
            r[row_map[i]].push_back(std::make_pair(col_map[j], result[i](i, j)));
          }
        }
      }
      */
    }
    sync();
  }

 private:
  string input, output;
  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_A_T;
  unordered_map<size_t, string> row_map, col_map;
  int blk_size = 0;
  std::vector<Eigen::MatrixXd> result;
}; // class cos_sim_sparse 

} // namespace paracel

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser pt(FLAGS_cfg_file);
  string input = pt.parse<string>("input");
  string output = pt.parse<string>("output");
  
  paracel::cos_sim_sparse solver(comm, FLAGS_server_info, input, output);
  solver.solve();
  solver.dump();
  return 0;
}
