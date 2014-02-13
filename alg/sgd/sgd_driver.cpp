/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#include <string>
#include <iostream>

#include <mpi.h>
#include <google/gflags.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "sgd.hpp"
#include "utils.hpp"

using namespace boost::property_tree;

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  ptree pt;
  json_parser::read_json(FLAGS_cfg_file, pt);
  std::string input = pt.get<std::string>("input");
  std::string output = pt.get<std::string>("output");
  double alpha = pt.get<double>("alpha");
  double beta = pt.get<double>("beta");
  int rounds = pt.get<int>("rounds");

  paracel::sgd sgd_solver(comm, FLAGS_server_info, input, output, rounds, alpha, beta); 
  std::cout << "hello sgd" << std::endl;
  //sgd_solver.solve();
  //sgd_solver.calc_loss();
  //sgd_solver.dump_result();

  return 0;
}
