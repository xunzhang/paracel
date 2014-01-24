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
#ifndef FILE_e91abf0f_fadd_f098_91ba_d9ed64bbee7f_HPP
#define FILE_e91abf0f_fadd_f098_91ba_d9ed64bbee7f_HPP

#include <cstdlib>
#include <ctime>

#include <zmq.hpp>

#include "utils/comm.hpp"
#include "utils/hash.hpp"
#include "utils/decomp.hpp"
#include "utils/ext_utility.hpp"
#include "utils/func_utility.hpp"

#include "paracel_types.hpp"

namespace paracel {

typedef paracel::dict_type<paracel::str_type, paracel::str_type> local_dict_type;

paracel::str_type get_hostnames_string(int srv_num) {
  paracel::str_type s;
  zmq::context_t context(2);
  zmq::socket_t sock(context, ZMQ_REP);
  paracel::str_type info = "tcp://*:" + paracel::default_port;
  sock.bind(info.c_str());
  for(int i = 0; i < srv_num; ++i) {
    zmq::message_t request;
    sock.recv(&request);
    paracel::str_type msg = paracel::str_type(static_cast<char *>(request.data()), request.size());
    s += msg;
    if(i != srv_num - 1) { 
      s += paracel::seperator; 
    }
    std::cout << "client: " << s << std::endl;
    zmq::message_t reply(4);
    std::memcpy((void *)reply.data(), "done", 4);
    sock.send(reply);
  }
  return s;
}

paracel::list_type<local_dict_type> 
get_hostnames_dict(const paracel::str_type & names) {
  paracel::list_type<local_dict_type> dl;
  auto lst = paracel::str_split(names, paracel::seperator);
  for(auto & item : lst) {
    local_dict_type d;
    auto l = paracel::str_split(item, ':');
    d["host"] = l[0];
    d["port"] = l[1];
    dl.push_back(std::move(d));
  }
  return dl;
}

static inline size_t random_size_t(size_t s, size_t e) {
  std::srand(std::time(0));
  return s + std::rand() % (e - s + 1);
}

static bool is_idle_port(size_t port) {}

static size_t gen_port() {
  size_t port = random_size_t(2014, 65535);
  while(!is_idle_port(port)) {
    port = random_size_t(2014, 65535);
  }
  return port;
}

paracel::list_type<size_t> get_ports() {
  paracel::list_type<size_t> ports_lst;
  for(int i = 0; i < paracel::threads_num; ++i) {
    ports_lst.emplace_back(std::move(gen_port()));
  }
  return ports_lst;
}

} // namespace paracel

#endif
