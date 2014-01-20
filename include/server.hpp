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
#include <thread>
#include <mutex>
#include <functional>

#include <zmq.hpp>

#include "paracel_types.hpp"
#include "utils.hpp"
#include "packer.hpp"
#include "kv_def.hpp"

namespace paracel {

std::mutex mutex;

static paracel::str_type local_parse_port(paracel::str_type && s) {
  auto l = paracel::str_split(std::move(s), ':');
  return std::move(l[2]);
}

static void rep_send(zmq::socket_t & sock, paracel::str_type & val) {
  zmq::message_t req(val.size());
  std::memcpy((void *)req.data(), &val[0], val.size());
  sock.send(req);
}

template <class V>
static void rep_pack_send(zmq::socket_t & sock, V & val) {
  paracel::packer<V> pk(val);
  paracel::str_type r;
  pk.pack(r);
  zmq::message_t req(r.size());
  std::memcpy((void *)req.data(), &r[0], r.size());
  sock.send(req);
}

// thread entry
void thrd_exec(zmq::socket_t & sock) {
  while(1) {
    zmq::message_t s;
    sock.recv(&s);
    auto scrip = paracel::str_type(static_cast<const char *>(s.data()), s.size());
    auto msg = paracel::str_split(scrip, paracel::seperator);
    paracel::packer<> pk;
    auto indicator = pk.unpack(msg[0]);
    std::cout << indicator << std::endl;
    paracel::str_type ret;
    mutex.lock();
    if(indicator == "contains") {
      auto key = pk.unpack(msg[1]);
      auto result = paracel::tbl_store.contains(key);
      rep_pack_send(sock, result);
    }
    if(indicator == "pull") {
      auto key = pk.unpack(msg[1]);
      paracel::str_type result;
      auto exist = paracel::tbl_store.get(key, result);
      rep_send(sock, result);
      //auto result = paracel::tbl_store.get(key);
      //rep_send(sock, *result);
    }
    if(indicator == "pull_multi") {
      paracel::packer<paracel::list_type<paracel::str_type> > pk_l; 
      auto key_lst = pk_l.unpack(msg[1]);
      auto result = paracel::tbl_store.get_multi(key_lst);
      rep_pack_send(sock, result);
    }
    if(indicator == "pullall") {
      auto dct = paracel::tbl_store.getall();
      //rep_pack_send(sock, dct);
    }
    if(indicator == "pullall_special") {
    }
    if(indicator == "push") {
      auto key = pk.unpack(msg[1]);
      paracel::tbl_store.set(key, msg[2]);
      int result = 1; rep_pack_send(sock, result);
    }
    if(indicator == "push_multi") {
    /*
      paracel::packer<paracel::dict_type<paracel::str_type, paracel::str_type> > pk_d;
      auto kv_pairs = pk_d.unpack(msg[1]);
      paracel::tbl_store.set_multi(kv_pairs);
      int result = 1; rep_pack_send(sock, result);
    */
    }
    if(indicator == "update") {}
    if(indicator == "remove") {
      auto key = pk.unpack(msg[1]);
      auto result = paracel::tbl_store.del(key);
      rep_pack_send(sock, result);
    }
    if(indicator == "remove_special") {}
    if(indicator == "clear") { 
      paracel::tbl_store.clean();
      bool result = true;
      rep_pack_send(sock, result);
    }
    mutex.unlock();
  }
}

// init_host is the hostname of starter
void init_thrds(const paracel::str_type & init_host) {

  zmq::context_t context(2);
  zmq::socket_t sock(context, ZMQ_REQ);
  paracel::str_type info = "tcp://" + init_host + ":" + paracel::default_port;
  sock.connect(info.c_str());
 
  char hostname[1024], freeport[1024]; size_t size = sizeof(freeport);
  // parameter server's hostname
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
  
  zmq::socket_t sock_t2(context, ZMQ_PULL);
  sock_t2.bind("tcp://*:*");
  sock_t2.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport)) + ",";
  
  zmq::socket_t sock_t3(context, ZMQ_PULL);
  sock_t3.bind("tcp://*:*");
  sock_t3.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport));

  zmq::message_t request(ports.size()); 
  std::memcpy((void *)request.data(), &ports[0], ports.size());
  sock.send(request);

  zmq::message_t reply;
  sock.recv(&reply);
  
  paracel::list_type<std::thread> threads;
  threads.push_back(std::thread(thrd_exec, std::ref(sock_t0)));
  threads.push_back(std::thread(thrd_exec, std::ref(sock_t1)));
  threads.push_back(std::thread(thrd_exec, std::ref(sock_t2)));
  threads.push_back(std::thread(thrd_exec, std::ref(sock_t3)));
  /*
  threads.push_back(std::thread(thrd_exec, std::move(sock_t0)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t1)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t2)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t3)));
  */
  for(auto & thrd : threads) {
    thrd.join();
  }
}

} // namespace paracel

#endif
