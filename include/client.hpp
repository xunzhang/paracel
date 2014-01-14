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

#include <dlfcn.h>

#include <cstring> // std::memcpy
#include <memory>

#include <zmq.hpp>

#include "paracel_types.hpp"
#include "utils/ext_utility.hpp"
#include "packer.hpp"

namespace paracel {

struct kvclt {
public:

  kvclt(paracel::str_type hostname, 
  	paracel::str_type ports) 
	  : host(hostname), context(1) {
    ports_lst = paracel::str_split(ports, ',');
    conn_prefix = "tcp://" + host + ":";
  }
 
  template <class K, class V>
  V pull(const K & key) {
    if(p_pull_sock == nullptr) {
      p_pull_sock.reset(create_req_sock(ports_lst[0]));
    }
    auto scrip = paste(paracel::str_type("pull"), key); // paracel::str_type
    V val;
    req_send_recv(*p_pull_sock, scrip, val);
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
    if(p_pull_multi_sock == nullptr) {
      p_pull_multi_sock.reset(create_req_sock(ports_lst[0]));
    }
    auto scrip = paste(paracel::str_type("pull_multi"), key_lst);
    paracel::list_type<V> val;
    req_send_recv(*p_pull_multi_sock, scrip, val);
    return val;
  }
  

  // TODO: all different types 
  template <class V>
  V pullall() {
    if(p_pullall_sock == nullptr) {
      p_pullall_sock.reset(create_req_sock(ports_lst[0]));
    }
    auto scrip = paste(paracel::str_type("pullall"));
    V val;
    req_send_recv(*p_pullall_sock, scrip, val);
    return val;
  }

  template <class V>
  V pullall_special(const paracel::str_type & so_filename = paracel::default_so_file) {
    if(p_pullall_sock == nullptr) {
      p_pullall_sock.reset(create_req_sock(ports_lst[0]));
    }
    auto scrip = paste(paracel::str_type(), so_filename);
    // TODO
  }
  
  template <class K, class V>
  int push(const K & key, const V & val) {
    if(p_push_sock == nullptr) {
      p_push_sock.reset(create_req_sock(ports_lst[1]));
    }
    auto scrip = paste(paracel::str_type("push"), key, val); 
    int stat;
    req_send_recv(*p_push_sock, scrip, stat);
    return stat;
  }
  
  template <class K, class V>
  int push_multi(const paracel::dict_type<K, V> & dict) {
    if(p_push_multi_sock == nullptr) {
      p_push_multi_sock.reset(create_req_sock(ports_lst[1]));
    }
    auto scrip = paste(paracel::str_type("push_multi"), dict);
    int stat;
    req_send_recv(*p_push_multi_sock, scrip, stat);
    return stat;
  }
  
  template <class K, class V>
  void update(const K & key, const V & val, 
  	const paracel::str_type & so_filename = paracel::default_so_file) {
    if(p_update_sock == nullptr) {
      p_update_sock.reset(create_push_sock(ports_lst[2]));
    }
    auto scrip = paste(paracel::str_type("update"), key, val, so_filename);
    push_send(*p_update_sock, scrip);
  }
  
  template <class K>
  void remove(const K & key) {
    if(p_remove_sock == nullptr) {
      p_remove_sock.reset(create_push_sock(ports_lst[3]));
    }
    auto scrip = paste(paracel::str_type("remove"), key);
    push_send(*p_remove_sock, scrip);
  }

  void remove_special(const paracel::str_type & so_filename = paracel::default_so_file) {
    if(p_remove_sock == nullptr) {
      p_remove_sock.reset(create_push_sock(ports_lst[3]));
    }
    auto scrip = paste(paracel::str_type("remove_special"), so_filename);
    push_send(*p_remove_sock, scrip);
  }

  void clear() {
    if(p_clear_sock == nullptr) {
      p_clear_sock.reset(create_push_sock(ports_lst[3]));
    }
    auto scrip = paste(paracel::str_type("clear"));
    push_send(*p_clear_sock, scrip);
  }

private:

  zmq::socket_t*
  create_req_sock(const paracel::str_type & port) {
    zmq::socket_t *p_sock = new zmq::socket_t(context, ZMQ_REQ);
    auto info = conn_prefix + port;
    p_sock->connect(info.c_str());
    return p_sock;
  }

  zmq::socket_t*
  create_push_sock(const paracel::str_type & port) {
    zmq::socket_t *p_sock = new zmq::socket_t(context, ZMQ_PUSH);
    auto info = conn_prefix + port;
    p_sock->connect(info.c_str());
    return p_sock;
  }

  // terminate function
  template<class T>
  paracel::str_type paste(const T & arg) {
    paracel::packer<T> pk(arg);
    paracel::str_type scrip;
    pk.pack(scrip);
    return scrip;
  }

  // use template T to do recursive variadic template(T must be paracel::str_type)
  template<class T, class ...Args>
  paracel::str_type paste(const T & op_str, const Args & ...args) { 
    paracel::packer<T> pk(op_str);
    paracel::str_type scrip;
    pk.pack(scrip); // pack to scrip
    return scrip + paracel::seperator + paste(args...); 
  }

  template <class V>
  void req_send_recv(const zmq::socket_t & sock, const paracel::str_type & scrip, V & val) {
    zmq::message_t req_msg(scrip.size());
    std::memcpy((void *)req_msg.data(), &scrip[0], scrip.size());
    sock.send(req_msg);
    zmq::message_t rep_msg;
    sock.recv(&rep_msg);
    paracel::packer<V> pk;
    val = pk.unpack(paracel::str_type(static_cast<char *>(rep_msg.data(), rep_msg.size())));
  }
 
  void push_send(const zmq::socket_t & sock, const paracel::str_type & scrip) {
    zmq::message_t push_msg(scrip.size());
    std::memcpy((void *)push_msg.data(), &scrip[0], scrip.size());
    sock.send(push_msg);
  }

private:
  paracel::str_type host;
  paracel::list_type<paracel::str_type> ports_lst;
  paracel::str_type conn_prefix;
  zmq::context_t context;
  
  std::unique_ptr<zmq::socket_t> p_pull_sock;
  std::unique_ptr<zmq::socket_t> p_pull_multi_sock;
  std::unique_ptr<zmq::socket_t> p_pullall_sock;
  std::unique_ptr<zmq::socket_t> p_push_sock;
  std::unique_ptr<zmq::socket_t> p_push_multi_sock;
  std::unique_ptr<zmq::socket_t> p_update_sock;
  std::unique_ptr<zmq::socket_t> p_remove_sock;
  std::unique_ptr<zmq::socket_t> p_clear_sock;
}; 

} // namespace paracel

#endif