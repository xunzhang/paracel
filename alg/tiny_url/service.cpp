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
#include <mpi.h>
#include <google/gflags.h>

#include "utils.hpp"
#include "tiny_url.hpp"

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser pt(FLAGS_cfg_file);
  std::string input1 = pt.parse<std::string>("input_original_URLs");
  std::string input2 = pt.parse<std::string>("input_short_URLs");
  std::string output = pt.parse<std::string>("output");
  paracel::tiny_url solver(comm, FLAGS_server_info, input1, input2, output);
  solver.solve_insert();
  solver.solve_retrieve();
  return 0;
}
