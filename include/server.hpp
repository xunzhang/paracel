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
#ifndef FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP 
#define FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>

#include <zmq.hpp>

#include "paracel_types.hpp"
#include "utils.hpp"
#include "packer.hpp"
#include "kv_def.hpp"
#include "proxy.hpp"

namespace paracel {

std::mutex mutex;

using update_result = paracel::update_result;
using filter_result = paracel::filter_result;

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

void kv_filter4pullall(const paracel::dict_type<paracel::str_type, paracel::str_type> & dct, 
	paracel::dict_type<paracel::str_type, paracel::str_type> & new_dct,
	filter_result filter_func) {
  for(auto & kv : dct) {
    if(filter_func(kv.first, kv.second)) {
      new_dct[kv.first] = kv.second;
    }
  }
}

void kv_filter4remove(const paracel::dict_type<paracel::str_type, paracel::str_type> & dct,
	filter_result filter_func) {
  for(auto & kv : dct) { 
    paracel::str_type v;
    auto key = kv.first;
    auto exist = paracel::tbl_store.get(key, v);
    if(!exist) {
      std::cerr << "kv_filter4remove: " << key << " is not exist." << '\n';
      abort();
    }
    if(filter_func(key, v)) {
      paracel::tbl_store.del(key);
    }
  }
}

void kv_update(const paracel::str_type & key, 
	const paracel::str_type & delta, 
	update_result update_func) {
  paracel::str_type val;
  auto exist = paracel::tbl_store.get(key, val);
  if(!exist) {
    std::cerr << "kv_update: " << key << " is not exist." << '\n';
    abort();
  }
  auto new_val = update_func(val, delta);
  paracel::tbl_store.set(key, new_val); 
}

// thread entry for ssp 
void thrd_exec_ssp(zmq::socket_t & sock) {
  paracel::packer<> pk;
  paracel::ssp_tbl.set("server_clock", 0);
  while(1) {
    zmq::message_t s;
    sock.recv(&s);
    auto scrip = paracel::str_type(static_cast<const char *>(s.data()), s.size());
    auto msg = paracel::str_split_by_word(scrip, paracel::seperator);
    auto indicator = pk.unpack(msg[0]);
    //std::cout << indicator << std::endl;
    if(indicator == "push_int") {
      auto key = pk.unpack(msg[1]);
      paracel::packer<int> pk_i;
      auto val = pk_i.unpack(msg[2]);
      paracel::ssp_tbl.set(key, val);
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "incr_int") {
      auto key = pk.unpack(msg[1]);
      if(paracel::startswith(key, "client_clock_")) {
        if(paracel::ssp_tbl.get(key)) {
	  paracel::ssp_tbl.incr(key, 1);
	} else {
	  paracel::ssp_tbl.set(key, 1);
	}
	if(paracel::ssp_tbl.get(key) >= paracel::ssp_tbl.get("worker_sz")) {
	  paracel::ssp_tbl.incr("server_clock", 1);
	  paracel::ssp_tbl.set(key, 0); 
	}
      }
      paracel::packer<int> pk_i;
      int delta = pk_i.unpack(msg[2]);
      paracel::ssp_tbl.incr(key, delta);
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "pull_int") {
      auto key = pk.unpack(msg[1]);
      int result = 0;
      paracel::ssp_tbl.get(key, result);
      rep_pack_send(sock, result);
    }
  }
}

// thread entry
void thrd_exec(zmq::socket_t & sock) {

  paracel::packer<> pk;
  //void *update_handler; 
  //void *pullall_special_handler;
  //void *remove_special_handler;
  update_result update_f;
  filter_result pullall_special_f;
  filter_result remove_special_f;
  
  auto dlopen_update_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_update_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_update_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  auto dlopen_pullall_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_pullall_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_pullall_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    pullall_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  auto dlopen_remove_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_remove_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_remove_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    remove_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  while(1) {
    zmq::message_t s;
    sock.recv(&s);
    auto scrip = paracel::str_type(static_cast<const char *>(s.data()), s.size());
    auto msg = paracel::str_split_by_word(scrip, paracel::seperator);
    auto indicator = pk.unpack(msg[0]);
    //std::cout << indicator << std::endl;
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
      if(!exist) {
        std::cerr << "while: " << key << " is not exist." << '\n';
        abort();
      }
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
      rep_pack_send(sock, dct);
    }
    if(indicator == "pullall_special") {
      if(msg.size() == 3) {
        // open request func
        auto file_name = pk.unpack(msg[1]);
        auto func_name = pk.unpack(msg[2]);
	dlopen_pullall_lambda(file_name, func_name);
	/*
        pullall_special_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
        if(!pullall_special_handler) {
          std::cerr << "Cannot open library: " << dlerror() << '\n';
	  return;
        }
        auto pullall_special_local = dlsym(pullall_special_handler, func_name.c_str());
        if(!pullall_special_local) {
          std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	  dlclose(pullall_special_handler);
	  return;
        }
        pullall_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) pullall_special_local;
        dlclose(pullall_special_handler);
        */
      } else {
        // work with registered mode
        if(!pullall_special_f) {
          std::cerr << "You must define filter in using this interface" << dlerror() << '\n';
	  abort();
        }
	//TODO
      }
      auto dct = paracel::tbl_store.getall();
      paracel::dict_type<paracel::str_type, paracel::str_type> new_dct;
      kv_filter4pullall(dct, new_dct, pullall_special_f);
      rep_pack_send(sock, new_dct);
    }
    if(indicator == "register_pullall_special") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_pullall_lambda(file_name, func_name);
      /*
      pullall_special_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
      if(!pullall_special_handler) {
        std::cerr << "Cannot open library: " << dlerror() << '\n';
	return;
      }
      auto pullall_special_local = dlsym(pullall_special_handler, func_name.c_str());
      if(!pullall_special_local) {
        std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	dlclose(pullall_special_handler);
	return;
      }
      pullall_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) pullall_special_local;
      dlclose(pullall_special_handler);
      */
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "register_remove_special") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_remove_lambda(file_name, func_name);
      /*
      remove_special_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
      if(!remove_special_handler) {
        std::cerr << "Cannot open library: " << dlerror() << '\n';
	return;
      }
      auto remove_special_local = dlsym(remove_special_handler, func_name.c_str());
      if(!remove_special_local) {
        std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	dlclose(remove_special_handler);
	return;
      }
      remove_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) remove_special_local;
      dlclose(remove_special_handler);
      */
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "register_update" || indicator == "register_bupdate") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_update_lambda(file_name, func_name);
      /*
      update_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
      if(!update_handler) {
        std::cerr << "Cannot open library: " << dlerror() << '\n';
	return;
      }
      auto update_local = dlsym(update_handler, func_name.c_str());
      if(!update_local) {
        std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	dlclose(update_handler);
	return;
      }
      update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) update_local;
      dlclose(update_handler);
      */
      if(indicator == "register_bupdate") {
        bool result = true; rep_pack_send(sock, result);
      }
    }
    if(indicator == "push") {
      auto key = pk.unpack(msg[1]);
      paracel::tbl_store.set(key, msg[2]);
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "push_multi") {
      paracel::packer<paracel::list_type<paracel::str_type> > pk_l;
      paracel::dict_type<paracel::str_type, paracel::str_type> kv_pairs;
      auto key_lst = pk_l.unpack(msg[1]);
      auto val_lst = pk_l.unpack(msg[2]);
      assert(key_lst.size() == val_lst.size());
      for(int i = 0; i < (int)key_lst.size(); ++i) {
        kv_pairs[key_lst[i]] = val_lst[i];
      }
      paracel::tbl_store.set_multi(kv_pairs);
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "update" || indicator == "bupdate") {
      if(msg.size() > 3) {
        if(msg.size() != 5) {
	  std::cerr << "Invalid invoke!!" << dlerror() << '\n';
	  abort();
	}
        // open request func
	auto file_name = pk.unpack(msg[3]);
	auto func_name = pk.unpack(msg[4]);
	dlopen_update_lambda(file_name, func_name);
	/*
        update_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
	if(!update_handler) {
	  std::cerr << "Cannot open library: " << dlerror() << '\n';
	  return;
	}
	auto update_local = dlsym(update_handler, func_name.c_str());
	if(!update_local) {
	  std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	  dlclose(update_handler);
	  return;
	}
	update_f = *(std::function<std::string(paracel::str_type, paracel::str_type)>*) update_local;
        dlclose(update_handler); // RTLD_NODELETE
        */
      } else {
        if(!update_f) {
	  dlopen_update_lambda("/mfs/user/wuhong/paracel/lib/default.so", "default_incr_d");
	  /*
          update_handler = dlopen("/mfs/user/wuhong/paracel/lib/default.so", RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
	  if(!update_handler) {
	    std::cerr << "Cannot open library: " << dlerror() << '\n';
	    return;
	  }
	  auto update_local = dlsym(update_handler, "default_incr_d");
	  if(!update_local) {
	    std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	    dlclose(update_handler);
	    return;
	  }
	  update_f = *(std::function<std::string(paracel::str_type, paracel::str_type)>*) update_local;
          dlclose(update_handler); // RTLD_NODELETE
          */
	}
      }
      auto key = pk.unpack(msg[1]);
      kv_update(key, msg[2], update_f);
      if(indicator == "bupdate") {
        bool result = true;
        rep_pack_send(sock, result);
      }
    }
    if(indicator == "remove") {
      auto key = pk.unpack(msg[1]);
      auto result = paracel::tbl_store.del(key);
      rep_pack_send(sock, result);
    }
    if(indicator == "remove_special") {
      if(msg.size() == 3) {
        // open request func
	auto file_name = pk.unpack(msg[1]);
	auto func_name = pk.unpack(msg[2]);
	dlopen_remove_lambda(file_name, func_name);
	/*
	remove_special_handler = dlopen(file_name.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
	if(!remove_special_handler) {
          std::cerr << "Cannot open library: " << dlerror() << '\n';
	  return;
	}
	auto remove_special_local = dlsym(remove_special_handler, func_name.c_str());
	if(!remove_special_local) {
          std::cerr << "Cannot load symbol: " << dlerror() << '\n';
	  dlclose(remove_special_handler);
	  return;
	}
        remove_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) remove_special_local;
	dlclose(remove_special_handler);
        */
      } else {
        if(!remove_special_f) {
          std::cerr << "You must define filter in using this interface" << dlerror() << '\n';
	  abort();
        }
      }
      auto dct = paracel::tbl_store.getall();
      kv_filter4remove(dct, remove_special_f);
      bool result = true; rep_pack_send(sock, result);
    }
    if(indicator == "clear") { 
      paracel::tbl_store.clean();
      bool result = true;
      rep_pack_send(sock, result);
    }
    mutex.unlock();
  }
}

// init_host is the hostname of starter
void init_thrds(const paracel::str_type & init_host, const paracel::str_type & init_port) {

  zmq::context_t context(2);
  zmq::socket_t sock(context, ZMQ_REQ);
  //paracel::str_type info = "tcp://" + init_host + ":" + paracel::default_port;
  paracel::str_type info = "tcp://" + init_host + ":" + init_port;
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
  
  zmq::socket_t sock_t3(context, ZMQ_REP);
  sock_t3.bind("tcp://*:*");
  sock_t3.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
  ports += local_parse_port(paracel::str_type(freeport)) + ",";
  
  zmq::socket_t sock_t4(context, ZMQ_REP);
  sock_t4.bind("tcp://*:*");
  sock_t4.getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
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
  threads.push_back(std::thread(thrd_exec_ssp, std::ref(sock_t4)));
  /*
  threads.push_back(std::thread(thrd_exec, std::move(sock_t0)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t1)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t2)));
  threads.push_back(std::thread(thrd_exec, std::move(sock_t3)));
  */
  for(auto & thrd : threads) {
    thrd.join();
  }
  zmq_ctx_destroy(context);
}

} // namespace paracel

#endif
