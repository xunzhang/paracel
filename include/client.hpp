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
#ifndef FILE_a68107bb_430d_eb9e_9cce_184c46bad4cd_HPP 
#define FILE_a68107bb_430d_eb9e_9cce_184c46bad4cd_HPP

#include <cstring> // std::memcpy

#include <zmq.hpp>

#include "paracel_types.hpp"
#include "utils/ext_utility.hpp"
#include "packer.hpp"

namespace paracel {

struct kvclt {
public:

  kvclt(paracel::str_type hostname, 
  	paracel::str_type ports) 
	  : host(hostname) {
    ports_lst = paracel::str_split(ports, ',');
    conn_prefix = "tcp://" + host + ":";
    // init zmq context
    zmq::context_t ctx(1);
    context = ctx; 
  }
  
  kvclt(paracel::str_type hostname, 
  	paracel::str_type ports) 
	  : host(hostname), context(ctx) {
    ports_lst = paracel::str_split(ports, ',');
    conn_prefix = "tcp://" + host + ":";
  }
 
  template <class K, class V>
  V pull(const K & key) {
    if(!pull_flag) {
      pull_sock = create_req_sock(ports_lst[0]);
      pull_flag = true;
    }
    auto scrip = paste(paracel::str_type("pull"), key); // paracel::str_type
    V val;
    req_send_recv(pull_sock, scrip, val);
    return val;
    /*
    zmq::message_t req_msg(scrip.size());
    std::memcpy((void *)req_msg.data(), &scrip[0], scrip.size());
    // send request scrip
    pull_sock.send(req_msg);

    // receive reply scrip
    zmq::message_t rep_msg;
    pull_sock.recv(&rep_msg);

    // unpack scrip
    paracel::packer<V> pk;
    return pk.unpack(paracel::str_type(static_cast<char *>(rep_msg.data(), rep_msg.size())));
    */
  }
 
  template <class K, class V>
  paracel::list_type<V> 
  pull_multi(const paracel::list_type<K> & key_lst) {
    if(!pull_multi_flag) {
      pull_multi_sock = create_req_sock(ports_lst[0]);
      pull_multi_flag = true;
    }
    auto scrip = paste(paracel::str_type("pull_multi"), key_lst);
    paracel::list_type<V> val;
    req_send_recv(pull_multi_sock, scrip, val);
    return val;
  }
  

  // TODO: different types 
  template <class V>
  V pullall() {
    if(!pullall_flag) {
      pullall_sock = create_req_sock(ports_lst[0]);
      pullall_flag = true;
    }
    auto scrip = paste(paracel::str_type("pullall"));
    V val;
    req_send_recv(pullall_sock, scrip, val);
    return val;
  }

  template <class K>
  V pullall_with_skey(const paracel::str_type & sprefix) {
    if(!pullall_multi_flag) {
      pull_multi_sock = create_req_sock(ports_lst[0]);
      pull_multi_flag = true;
    }
    auto scrip = paste(paracel::str_type("pull_skey"), sprefix);
    V val;
    req_send_recv(pull_multi_sock, scrip, val);
    return val;
  }
  
  template <class V>
  V pullall_with_sval(const paracel::str_type & vprefix) {
    if(!pullall_multi_flag) {
      pull_multi_sock = create_req_sock(ports_lst[0]);
      pull_multi_flag = true;
    }
    auto scrip = paste(paracel::str_type("pull_sval"), vprefix);
    V val;
    req_send_recv(pull_multi_sock, scrip, val);
    return val;
  }
  
  template <class K, class V>
  int push(const K & key, const V & val) {
    if(!push_flag) {
      push_sock = create_req_sock(ports_lst[1]);
      push_flag = true;
    }
    auto scrip = paste(paracel::str_type("push"), key, val); 
    int stat;
    req_send_recv(push_sock, scrip, stat);
    return stat;
  }
  
  template <class K, class V>
  void push_multi(const paracel::dict_type<K, V> & dict) {
    if(!push_multi_flag) {
      push_multi_sock = create_req_sock(ports_lst[1]);
      push__multi_flag = true;
    }
    auto scrip = paste(paracel::str_type("push_multi"), dict);
    int stat;
    req_send_recv(push_multi_sock, scrip, stat);
    return stat;
  }
  
  template <class K, class V, class Func>
  void update() {}
  
  template <class K>
  void remove(const K & key) {
    if(!remove_flag) {
      remove_sock = create_push_sock(ports_lst[3]);
      remove_flag = true;
    }
    auto scrip = paste(paracel::str_type("remove"), key);
    push_send(remove_sock, scrip);
  }

  void remove_with_skey(const paracel::str_type & prefix) {}

  void remove_with_sval() {}

  void clear() {
    if(!clear_flag) {
      clear_sock = create_push_sock(ports_lst[3]);
      clear_flag = true;
    }
    auto scrip = paste(paracel::str_type("clear"));
    push_send(clear_sock, scrip);
  }

private:

  zmq::socket_t 
  create_req_sock(const paracel::str_type & port) {
    zmq::socket_t sock(context, ZMQ.REQ);
    sock.connect(conn_prefix + port);
    return sock
  }

  zmq::socket_t 
  create_push_sock(const paracel::str_type & port) {
    zmq::socket_t sock(context, ZMQ.PUSH);
    sock.connect(conn_prefix + port);
    return sock;
  }
  
  // terminate function for recursive variadic template
  template<class T>
  paracel::str_type paste(const T & arg) {
    paracel::packer<T> pk(arg);
    paracel::str_type scrip;
    pk.pack(scrip);
    return scrip;
  }

  // use template T to do recursive variadic(T must be paracel::str_type)
  template<class T, class ...Args>
  T paste(const T & op_str, const Args & ...args) { 
    paracel::packer<T> pk(op_str);
    T scrip;
    pk.pack(scrip); // pack to scrip
    return scrip + paracel::seperator + paste(args...); 
  }

  template <class V>
  void req_send_recv(zmq::socket_t & sock, const paracel::str_type & scrip, V & val) {
    zmq::message_t req_msg(scrip.size());
    std::memcpy((void *)req_msg.data(), &scrip[0], scrip.size());
    sock.send(req_msg);
    zmq::message_t rep_msg;
    sock.recv(&rep_msg);
    paracel::packer<V> pk;
    val = pk.unpack(paracel::str_type(static_cast<char *>(rep_msg.data(), rep_msg.size())));
  }
 
  void push_send(zmq::socket_t & sock, const paracel::str_type & scrip) {
    zmq::message_t push_msg(scrip.size());
    std::memcpy((void *)push_msg.data(), &scrip[0], scrip.size());
    sock.send(push_msg);
  }

private:
  bool pull_flag = false;
  bool pull_multi_flag = false;
  bool pullall_flag = false;
  bool push_flag = false;
  bool push_multi_flag = false;
  bool update_flag = false;
  bool remove_flag = false;
  bool clear_flag = false;
  paracel::str_type host;
  paracel::list_type<paracel::str_type> ports_lst;
  paracel::str_type conn_prefix;
  zmq::context_t context;
  zmq::socket_t pull_sock;
  zmq::socket_t pull_multi_sock;
  zmq::socket_t pullall_sock;
  zmq::socket_t push_sock;
  zmq::socket_t push_multi_sock;
  zmq::socket_t update_sock;
  zmq::socket_t remove_sock;
  zmq::socket_t clear_sock;
}; 

} // namespace paracel

#endif
