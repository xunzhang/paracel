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

#include <string>
#include <iostream>

#include <mpi.h>
#include <google/gflags.h>

#include "mf.hpp"
#include "utils.hpp"

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser pt(FLAGS_cfg_file);
  std::string input = pt.parse<std::string>("input");
  std::string output = pt.parse<std::string>("output");
  int k = pt.parse<int>("k");
  double alpha = pt.parse<double>("alpha");
  double beta = pt.parse<double>("beta");
  int rounds = pt.parse<int>("rounds");
  int limit_s = pt.parse<int>("limit_s");
  
  paracel::matrix_factorization mf_solver(comm, FLAGS_server_info, input, output, "ipm", k, rounds, alpha, beta, false, limit_s, true);
  mf_solver.solve();
  std::cout << mf_solver.cal_rmse() << std::endl;
  mf_solver.dump_result();
  return 0;
}
