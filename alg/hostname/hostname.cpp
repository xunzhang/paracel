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

#include <unistd.h>
#include <string>
#include <iostream>

#include <mpi.h>
#include <google/gflags.h>

#include "utils.hpp"
#include "ps.hpp"

namespace paracel {

class hostname : public paracel::paralg {
 
 public:
  hostname(paracel::Comm comm, 
           std::string hosts_dct_str, 
           std::string _output) : paracel::paralg(hosts_dct_str, comm, _output) {}

  void solve() {
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    std::string hn = hostname;
    paracel_bupdate("names", hn, "/mfs/user/wuhong/paracel/local/lib/libhostname_update.so", "hostname_updater");
    sync();
    if(get_worker_id() == 0) {
      names = paracel_read<std::string>("names");
      std::cout << names << std::endl;
    }
  }

private:
  std::string names;

};

} // namespace paracel

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::hostname obj(comm, FLAGS_server_info, "/mfs/user/wuhong/paracel/data/hostname_result/");
  obj.solve();
  return 0;
}
