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
#include <google/gflags.h>
#include <msgpack.hpp>

#include "paracel_types.hpp"
#include "server.hpp"

DEFINE_string(init_host, "balin", "host name of start node\n");

int main(int argc, char *argv[])
{
  google::SetUsageMessage("[options]\n\
  			--hostname\tdefault: balin\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  paracel::init_thrds(FLAGS_hostname);
  return 0;
}
