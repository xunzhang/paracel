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
#include <google/gflags.h>

//#include "kmeans.hpp"
#include "sc.hpp"
#include "utils.hpp"

using std::string;
using std::cout;
using std::endl;

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser jp(FLAGS_cfg_file);
  string input = jp.parse<string>("input");
  string output = jp.parse<string>("output");
  string type = jp.parse<string>("type");
  int k = jp.parse<int>("kclusters");
  int rounds = jp.parse<int>("rounds");
  int limit_s = jp.parse<int>("limit_s");
  /*
  paracel::kmeans solver(comm, FLAGS_server_info, input, output, type, k, rounds, limit_s);
  solver.solve();
  solver.dump_result();
  */
  paracel::spectral_clustering solver(comm, FLAGS_server_info, input, output, false, rounds, 0);
  solver.solve();
  return 0;
}
