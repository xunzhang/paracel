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
#include <google/gflags.h>

#include "cbr.hpp"
#include "utils.hpp"

using std::string;

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-server.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser jp(FLAGS_cfg_file);
  string input_rating = jp.parse<string>("input_rating");
  string input_miu = jp.parse<string>("input_miu");
  string input_ubias = jp.parse<string>("input_ubias");
  string input_ibias = jp.parse<string>("input_ibias");
  string input_ufac = jp.parse<string>("input_ufac");
  string input_ifac = jp.parse<string>("input_ifac");
  string output = jp.parse<string>("output");
  double alpha = jp.parse<double>("alpha");
  double beta = jp.parse<double>("beta");
  int rounds = jp.parse<int>("rounds");
  int limit_s = jp.parse<int>("limit_s");

  paracel::content_base_recommendation 
      cbr_solver(comm, 
                 FLAGS_server_info, 
                 input_rating, 
                 input_miu, 
                 input_ubias, 
                 input_ibias, 
                 input_ufac, 
                 input_ifac, 
                 output, 
                 rounds, 
                 alpha, 
                 beta,
                 limit_s);
  cbr_solver.solve();
  cbr_solver.dump_result();
  return 0;
}
