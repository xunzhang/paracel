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
#ifndef FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP 
#define FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP

#include <unistd.h>

#include <zmq.hpp>

#include "paracel_types.hpp"
#include "utils.hpp"

namespace paracel {

static paracel::str_type local_parse_port(paracel::str_type && s) {
  auto l = paracel::str_split(std::move(s), ':');
  return std::move(l[2]);
}

void init_thrds(const paracel::str_type & init_host) {

  zmq::context_t context(2);
  zmq::socket_t sock(context, ZMQ_REQ);
  paracel::str_type info = "tcp://" + init_host + ":" + paracel::default_port;
  sock.connect(info.c_str());
 
  char hostname[1024], freeport[1024];
  size_t size = sizeof(freeport);
  // get hostname
  gethostname(hostname, sizeof(hostname));
  
  paracel::str_type ports = hostname;
  ports += ":";
  // create sock in every thrd
  zmq::socket_t sock_t0(context, ZMQ_REP);
  sock_t0.bind("tcp://*:*");
  sock_t0.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(std::move(paracel::str_type(freeport))) + ",";

  zmq::socket_t sock_t1(context, ZMQ_REP);
  sock_t1.bind("tcp://*:*");
  sock_t1.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport)) + ",";
  
  zmq::socket_t sock_t2(context, ZMQ_REP);
  sock_t2.bind("tcp://*:*");
  sock_t2.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport)) + ",";
  
  zmq::socket_t sock_t3(context, ZMQ_REP);
  sock_t3.bind("tcp://*:*");
  sock_t3.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport));
   
  zmq::message_t request(4096); 
  std::memcpy((void *)request.data(), &ports[0], ports.size());
  std::cout << "server: " << ports << std::endl;
  sock.send(request);

  zmq::message_t reply;
  sock.recv(&reply);
}

} // namespace paracel

#endif
