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
#ifndef FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP
#define FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP

#include "paracel_types.hpp"
#include "utils.hpp" // paracel::Comm
#include "client.hpp"
#include "ring.hpp"

namespace paracel {
namespace ps {

class parasrv{

using l_type = paracel::list_type<paracel::kvclt>;
using dl_type = paracel::list_type<paracel::dict_type<paracel::str_type, paracel::str_type> >; 

public:
  parasrv(paracel::str_type hosts_dct_str) {
    // init dct_lst
    dct_lst = get_hostnames_dict(hosts_dct_str);
    // init srv_sz
    srv_sz = dct_lst.size();
    // init kvm
    for(auto & srv : dct_lst) {
      paracel::kvclt kvc(srv["node"], srv["ports"]);
      kvm.push_back(std::move(kvc));
    }
    // init servers
    for(auto i = 0; i < srv_sz; ++i) {
      servers.push_back(i);
    }
    // init hashring
    p_ring = new paracel::ring<int>(servers);
  }

  virtual ~parasrv() {
    delete p_ring;
  }

private:
  dl_type dct_lst;
  size_t srv_sz = 1;
  l_type kvm;
  paracel::list_type<int> servers;
  paracel::ring<int> *p_ring;
};


class paralg : public parasrv {
public:
  paralg(paracel::str_type hosts_dct_str, 
  	paracel::Comm comm,
  	size_t n_worker,
	size_t o_rounds = 1, size_t o_limit_s = 0) : 
	  parasrv(hosts_dct_str), 
	  worker_comm(comm),
	  nworker(n_worker), 
	  rounds(o_rounds), limit_s(o_limit_s) {
  }
  virtual ~paralg() {}

private:
  size_t nworker = 1;
  size_t rounds = 0;
  size_t limit_s = 0;
  paracel::Comm worker_comm;
};

} // namespace ps
} // namespace paracel

#endif
