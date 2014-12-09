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
#ifndef FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP
#define FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP

#include <assert.h>
#include <dlfcn.h>
#include <set>
#include <fstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <queue>

#include <boost/filesystem.hpp>
#include <boost/any.hpp>

#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>

#include "paracel_types.hpp"
#include "client.hpp"
#include "utils.hpp"
#include "graph.hpp"
#include "load.hpp"
#include "ring.hpp"
#include "packer.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

class paralg {

 private:
  void init_output(const paracel::str_type & folder) {
    // create output folder
    if(worker_comm.get_rank() == 0) {
      boost::filesystem::path path(folder);
      if(boost::filesystem::exists(path)) {
        boost::filesystem::remove_all(path);
      }
      boost::filesystem::create_directory(path);
    }
  }

  // TODO: abstract update_f to para
  void load_update_f(const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if(!handler) {
      std::cerr << "Cannot open library: " << dlerror() << '\n';
      return;
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol: " << dlerror() << '\n';
      dlclose(handler);
      return;
    }
    update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  }

 public:
  // interface for direct usage
  paralg(paracel::Comm comm,
         paracel::str_type _output,
         int _rounds) : worker_comm(comm), 
                        output(_output), 
                        rounds(_rounds) {
    init_output(_output);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
    ps_obj = NULL;
  }

  paralg(paracel::str_type hosts_dct_str, 
         paracel::Comm comm,
         paracel::str_type _output,
         int _rounds = 1,
         int _limit_s = 0,
         bool _ssp_switch = false) : worker_comm(comm),
                                    output(_output),
                                    nworker(comm.get_size()),
                                    rounds(_rounds),
                                    limit_s(_limit_s),
                                    ssp_switch(_ssp_switch) {
    ps_obj = new parasrv(hosts_dct_str);
    init_output(_output);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
    if(worker_comm.get_rank() == 0) {
      paracel::str_type key = "worker_sz";
      (ps_obj->kvm[clock_server]).push_int(key, worker_comm.get_size());
    }
    worker_comm.sync();
  }

  virtual ~paralg() {
    if(ps_obj) {
      delete ps_obj;
    }
  }

  void set_decomp_info(const paracel::str_type & pattern) {
    int np = worker_comm.get_size();
    paracel::npfactx(np, npx, npy);
    if(pattern == "fsmap") {
      paracel::npfact2d(np, npx, npy);
    }
    if(pattern == "smap") {
      paracel::npfacty(np, npx, npy); 
    }
  }

  template <class T>
  paracel::list_type<paracel::str_type> paracel_loadall(const T & fn) {
    auto fname_lst = paracel::expand(fn);
    paracel::list_type<paracel::str_type> lines;
    for(auto & fname : fname_lst) {
      std::ifstream f(fname, std::ios::binary);
      if(!f) { throw std::runtime_error("paracel error in paracel_loadall: loader reading failed."); }
      paracel::str_type l;
      while(std::getline(f, l)) {
        lines.push_back(l);
      }
      f.close();
    }
    return lines;
  }

  template <class T, class F>
  void paracel_sequential_loadall(const T & fn, F & func) {
    auto fname_lst = paracel::expand(fn);
    auto loads = paracel::files_partition(fname_lst, get_worker_size(), "linesplit");
    // sequential_load
    for(size_t i = 0; i < get_worker_size(); ++i) {
      paracel::list_type<paracel::str_type> result;
      while(loads[i]) {
        auto lines = loads[i].get();
        result.push_back(lines);
        loads[i]();
      }
      func(result);
    }
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
               parser_type & parser,
               const paracel::str_type & pattern = "linesplit",
               bool mix_flag = false) {
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    set_decomp_info(pattern);
    //assert(lines.size() != 0);
    return lines;
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
               const paracel::str_type & pattern = "linesplit",
               bool mix_flag = false) {
    parser_type parser;
    return paralg::paracel_load(fn, parser, pattern, mix_flag);	
  }

  // only support paracel::digraph<paracel::default_id_type> and paracel::digraph<std::string>
  template <class T, class G>
  void paracel_load_as_graph(paracel::digraph<G> & grp,
                             const T & fn, 
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    // TODO: check pattern 
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    sync();
    // create graph 
    ld.create_graph(lines, grp);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  // only support paracel::bigraph<paracel::default_id_type> and paracel::bigraph<std::string>
  template <class T, class G>
  void paracel_load_as_graph(paracel::bigraph<G> & grp,
                             const T & fn,
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    // TODO: check pattern 
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    sync();
    // create graph 
    ld.create_graph(lines, grp);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  template <class T>
  void paracel_load_as_graph(paracel::bigraph_continuous & grp,
                             paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & row_map,
                             paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & col_map,
                             const T & fn,
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    // TODO: check pattern 
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    sync();
    // create graph 
    ld.create_graph(lines, grp, row_map, col_map);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              paracel::dict_type<size_t, paracel::str_type> & row_map,
                              paracel::dict_type<size_t, paracel::str_type> & col_map,
                              paracel::dict_type<size_t, int> & degree_map,
                              paracel::dict_type<size_t, int> & col_degree_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    // TODO: check pattern
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.load();
    sync();
    // create sparse matrix
    ld.create_matrix(lines, blk_mtx, row_map, col_map, degree_map, col_degree_map);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }
  
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              paracel::dict_type<size_t, paracel::str_type> & row_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    paracel::dict_type<size_t, paracel::str_type> col_map;
    paracel::dict_type<size_t, int> degree_map, col_degree_map;
    paralg::paracel_load_as_matrix(blk_mtx, 
                                   row_map, col_map, degree_map, col_degree_map, 
                                   fn, parser, pattern, mix_flag);
  }

  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              paracel::dict_type<size_t, paracel::str_type> & row_map,
                              paracel::dict_type<size_t, paracel::str_type> & col_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    paracel::dict_type<size_t, int> degree_map, col_degree_map;
    paralg::paracel_load_as_matrix(blk_mtx, 
                                   row_map, col_map, degree_map, col_degree_map, 
                                   fn, parser, pattern, mix_flag);
  }

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    return paralg::paracel_load_as_matrix(blk_mtx, 
                                          rm, cm, dm, col_dm, 
                                          fn, parser, pattern, mix_flag);
  }

  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
                              paracel::dict_type<size_t, paracel::str_type> & row_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {

    // TODO: check pattern
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    sync();
    // create sparse matrix
    ld.create_matrix(lines, blk_dense_mtx, row_map);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    paralg::paracel_load_as_matrix(blk_dense_mtx, rm, fn, parser, pattern, mix_flag);	
  }

  // put where you want to control iter with ssp
  void iter_commit() {
    paracel::str_type clock_key;
    if(limit_s == 0) {
      clock_key = "client_clock_0";
    } else {
      clock_key = "client_clock_" + std::to_string(clock % limit_s);
    }
    ps_obj->kvm[clock_server].incr_int(paracel::str_type(clock_key), 1); // value 1 is not important
    clock += 1;
    if(clock == total_iters) {
      ps_obj->kvm[clock_server].incr_int(paracel::str_type("worker_sz"), -1);
    }
  }

  void get_decomp_info(int & x, int & y) {
    x = npx;
    y = npy;
  }

  bool paracel_register_update(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    load_update_f(file_name, func_name);
    //auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_update(file_name, func_name);
    }
    return r;
  }

  bool paracel_register_bupdate(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    //local_update_f(file_name, func_name);
    //auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_bupdate(file_name, func_name);
    }
    return r;
  }

  bool paracel_register_read_special(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    //auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_pullall_special(file_name, func_name);
    }
    return r;
  }

  bool paracel_register_remove_special(const paracel::str_type & file_name, const paracel::str_type & func_name) {
    //auto rg = ps_obj->p_ring;
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_remove_special(file_name, func_name);
    }
    return r;
  }

  template <class V>
  bool paracel_read(const paracel::str_type & key, V & val, int replica_id = -1) {
    if(ssp_switch) {
      /*
         std::cout << "--------------" << std::endl;
         std::cout << get_worker_id() << "stale_cache:" << stale_cache << std::endl;
         std::cout << get_worker_id() << "clock" << clock << std::endl;
         std::cout << "--------------" << std::endl;
      */  
      if(clock == 0 || clock == total_iters) { // check total_iters for last pull
        cached_para[key] = boost::any_cast<V>(ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      } else if(stale_cache + limit_s > clock) {
        // cache hit
        val = boost::any_cast<V>(cached_para[key]);
      } else {
        // cache miss
        // pull from server until leading slowest less than s clocks
        while(stale_cache + limit_s < clock) {
          stale_cache = ps_obj->kvm[clock_server].pull_int(paracel::str_type("server_clock"));
        }
        cached_para[key] = boost::any_cast<V>(ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      }
      return true;
    }
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull(key, val); 
  }

  template <class V>
  V paracel_read(const paracel::str_type & key, int replica_id = -1) {
    V val;
    paracel_read(key, val, replica_id);
    return val;
  }

  template<class V>
  bool paracel_read_multi(const paracel::list_type<paracel::str_type> & keys,
                          paracel::list_type<V> & vals) {
    if(ssp_switch) {
      // TODO
    }
    paracel::dict_type<paracel::str_type, size_t> indx_map;
    paracel::list_type<paracel::list_type<paracel::str_type> > lst_lst(ps_obj->srv_sz);
    vals.resize(keys.size());
    for(size_t k = 0; k < keys.size(); ++k) {
      lst_lst[ps_obj->p_ring->get_server(keys[k])].push_back(keys[k]);
      indx_map[keys[k]] = k;
    }
    for(size_t k = 0; k < lst_lst.size(); ++k) {
      auto lst = ps_obj->kvm[k].pull_multi<V>(lst_lst[k]);
      for(size_t i = 0; i < lst.size(); ++i) {
        vals[indx_map[lst_lst[k][i]]] = lst[i];
      }
    }
    return true;
  }

  // TODO
  template<class V>
  paracel::dict_type<paracel::str_type, V> paracel_readall() {
    paracel::dict_type<paracel::str_type, V> d;
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      auto tmp = ps_obj->kvm[indx].pullall<V>();
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
    }
    return d;
  }
  
  template<class V, class F>
  void paracel_readall_handle(F & func) {
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      paracel::dict_type<paracel::str_type, V> d;
      auto tmp = ps_obj->kvm[indx].pullall<V>();
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
      func(d);
    }
  }

  template <class V>
  paracel::dict_type<paracel::str_type, V>
  paracel_read_special(const paracel::str_type & file_name,
                       const paracel::str_type & func_name) {
    paracel::dict_type<paracel::str_type, V> d;
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      auto tmp = ps_obj->kvm[indx].pullall_special<V>(file_name, func_name);
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
    }
    return d;
  }

  template <class V, class F>
  void paracel_read_special_handle(const paracel::str_type & file_name,
                              const paracel::str_type & func_name,
                              F & func) {
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      paracel::dict_type<paracel::str_type, V> d;
      auto tmp = ps_obj->kvm[indx].pullall_special<V>(file_name, func_name);
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
      func(d);
    }
  }

  /*
   * risk: params in server must be the same type
   */
  template <class T>
  void paracel_read_topk(int k, paracel::list_type<
                              std::pair<paracel::str_type, T> 
                              > & result) {
    using min_heap = std::priority_queue<std::pair<paracel::str_type, T>, 
                                        std::vector<std::pair<paracel::str_type, T> >, 
                                        min_heap_cmp<T> >;
    min_heap tmplst;
    auto handler = [&] (const paracel::dict_type<paracel::str_type, T> & d) {
      for(auto & kv : d) {
        auto node = paracel::heap_node<paracel::str_type, T>(kv.first, kv.second);
        tmplst.push(node.val);
        if((int)tmplst.size() > k) {
          tmplst.pop();
        }
      }
    }; // handler
    paracel_readall_handle<T>(handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first, tmplst.top().second));
      tmplst.pop();
    }
    std::reverse(result.begin(), result.end());
  }

  template <class T>
  void paracel_read_topk_with_key_filter(int k, 
                                         paracel::list_type<
                                          std::pair<paracel::str_type, T>
                                          > & result,
                                         const paracel::str_type & file_name,
                                         const paracel::str_type & func_name) {
    using min_heap = std::priority_queue<std::pair<paracel::str_type, T>,
                                        std::vector<std::pair<paracel::str_type, T> >,
                                        min_heap_cmp<T> >;
    min_heap tmplst;
    auto handler = [&] (const paracel::dict_type<paracel::str_type, T> & d) {
      for(auto & kv : d) {
        auto node = paracel::heap_node<paracel::str_type, T>(kv.first, kv.second);
        tmplst.push(node.val);
        if((int)tmplst.size() > k) {
          tmplst.pop();
        }
      }
    }; // handler
    paracel_read_special_handle<T>(file_name, func_name, handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first, tmplst.top().second));
      tmplst.pop();
    }
    std::reverse(result.begin(), result.end());
  }
  
  template <class V>
  bool paracel_write(const paracel::str_type & key, const V & val, bool replica_flag = true) {
    auto indx = ps_obj->p_ring->get_server(key);
    if(ssp_switch) {
      cached_para[key] = boost::any_cast<V>(val);
    }
    return (ps_obj->kvm[indx]).push(key, val);
  }

  bool paracel_write(const paracel::str_type & key, const char* val, bool replica_flag = true) {
    paracel::str_type v = val;
    return paralg::paracel_write(key, v); 
  }

  // TODO: package
  template <class V>
  bool paracel_write_multi(const paracel::dict_type<paracel::str_type, V> & dct) {
    if(ssp_switch) {
      for(auto & kv : dct) {
        cached_para[kv.first] = boost::any_cast<V>(kv.second);
      }
    }
    bool r = true;
    paracel::list_type<paracel::dict_type<paracel::str_type, V> > dct_lst(ps_obj->srv_sz);
    for(auto & kv : dct) {
      dct_lst[ps_obj->p_ring->get_server(kv.first)][kv.first] = kv.second;
    }
    for(size_t k = 0; k < dct_lst.size(); ++k) {
      if(dct_lst[k].size() != 0) {
        if(ps_obj->kvm[k].push_multi(dct_lst[k]) == false) {
          r = false;
        }
      }
    }
    return r;
  }

  template <class V>
  void paracel_update(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    if(ssp_switch) {
      if(!update_f) {
        // load default updater

        load_update_f("/mfs/user/wuhong/paracel/lib/default.so", "default_incr_d");
      }
      V val = boost::any_cast<V>(cached_para[key]);
      // pack<V> val to v & delta to d
      paracel::packer<V> pk1(val), pk2(delta);
      paracel::str_type v, d; pk1.pack(v); pk2.pack(d);
      auto nv = update_f(v, d);
      // unpack<V> nv
      V nval = pk1.unpack(nv);
      cached_para[key] = boost::any_cast<V>(nval);
    }
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key, delta);
  }

  void paracel_update(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_update(key, d);
  }

  template <class V>
  void paracel_bupdate(const paracel::str_type & key, const V & delta, bool replica_flag = true) {
    int indx = ps_obj->p_ring->get_server(key);
    ps_obj->kvm[indx].bupdate(key, delta);
    if(ssp_switch) {
      // update local cache
      cached_para[key] = boost::any_cast<V>(ps_obj->kvm[indx].pull<V>(key));
    }
  }

  void paracel_bupdate(const paracel::str_type & key, const char* delta, bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_bupdate(key, d, replica_flag);
  }

  template <class V>
  void paracel_bupdate(const paracel::str_type & key, 
                       const V & delta,
                       const paracel::str_type & file_name, 
                       const paracel::str_type & func_name,
                       bool replica_flag = true) {
    int indx = ps_obj->p_ring->get_server(key);
    ps_obj->kvm[indx].bupdate(key, delta, file_name, func_name);
    if(ssp_switch) {
      // update local cache
      cached_para[key] = boost::any_cast<V>(ps_obj->kvm[indx].pull<V>(key));
    }
  }

  void paracel_bupdate(const paracel::str_type & key, 
                       const char* delta, 
                       const paracel::str_type & file_name, 
                       const paracel::str_type & func_name,
                       bool replica_flag = true) {
    paracel::str_type d = delta;
    paralg::paracel_bupdate(key, d, file_name, func_name, replica_flag);
  }

  /*
  template <class V>
  void paracel_update_default(const paracel::str_type & key, const V & v_or_delta) {
    if(paracel_contains(key)) {
      paracel_bupdate(key, v_or_delta);
    } else {
      paracel_write(key, v_or_delta);
    }
  }
  */

  // TODO
  template <class V>
  void paracel_bupdate_multi(const paracel::list_type<paracel::str_type> & key,
                             const paracel::list_type<V> & delta,
                             const paracel::str_type & file_name,
                             const paracel::str_type & func_name) {}

  // set invoke cnts
  void set_total_iters(int n) {
    total_iters = n;
  }

  inline size_t get_worker_id() {
    return worker_comm.get_rank();
  }

  inline size_t get_worker_size() {
    return worker_comm.get_size();
  }

  inline size_t get_server_size() {
    return ps_obj->srv_sz;
  }

  void sync() {
    worker_comm.sync();
  }

  /**
   * Never called in if(rank == 0) clause because dup will hang.
   */
  paracel::Comm get_comm() {
    return worker_comm;
  }

  boost::any get_cache() {
    return cached_para;
  }

  bool is_cached(const paracel::str_type & key) {
    return cached_para.find(key) != cached_para.end();
  }

  template <class V>
  V get_cache(const paracel::str_type & key) {
    return boost::any_cast<V>(cached_para.at(key));
  }

  bool paracel_contains(const paracel::str_type & key) {
    auto indx = ps_obj->p_ring->get_server(key);
    return (ps_obj->kvm[indx]).contains(key);
  }

  bool paracel_remove(const paracel::str_type & key) {
    auto indx = ps_obj->p_ring->get_server(key);
    return ps_obj->kvm[indx].remove(key);
  }

  bool paracel_remove_multi(const paracel::list_type<paracel::str_type> & key_lst) {
    // TODO
    return true;
  }

  // TODO
  template <class V>
  void dump_line_as_vector() {}

  // buggy
  template <class T>
  void files_merge(const T & fn, 
                   const paracel::str_type & prefix = "result_") {
    auto fname_lst = paracel::expand(fn);
    std::ofstream os;
    os.open(paracel::todir(output) + prefix + "merge", std::ofstream::app);
    for(auto & fname : fname_lst) {
      std::ifstream f(fname, std::ios::binary);
      if(!f) { throw std::runtime_error("paracel error in files_merge: open reading failed."); }
      paracel::str_type l;
      while(std::getline(f, l)) {
        os << l << '\n';
      }
      f.close();
    }
    os.close();
  }

  // TODO
  template <class V>
  void paracel_dump_vector(const paracel::list_type<V> & data,
                           const paracel::dict_type<size_t, paracel::str_type> & id_map,
                           const paracel::str_type & filename = "result_",
                           const paracel::str_type & sep = ",",
                           bool merge = false) {}

  template <class V>
  void paracel_dump_vector(const paracel::list_type<V> & data, 
                           const paracel::str_type & filename = "result_",
                           const paracel::str_type & sep = ",", bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()), std::ofstream::app);
    for(size_t i = 0; i < data.size() - 1; ++i) {
      os << std::to_string(data[i]) << sep;
    }
    os << std::to_string(data[data.size() - 1]) << '\n';
    os.close();
    if(merge && get_worker_id() == 0) {
      sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }
  
  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, int> & data,
                         const paracel::str_type & filename = "result_",
                         bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()), std::ofstream::app);
    for(auto & kv : data) {
      os << kv.first << '\t' << kv.second << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, double> & data,
                         const paracel::str_type & filename = "result_",
                         bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()), std::ofstream::app);
    for(auto & kv : data) {
      os << kv.first << '\t' << kv.second << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  template <class T, class P>
  void paracel_dump_dict(const paracel::dict_type<
                         T, paracel::list_type<P> > & data, 
                         const paracel::str_type & filename = "result_",
                         bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()), std::ofstream::app);
    for(auto & kv : data) {
      os << kv.first << '\t';
      for(size_t i = 0; i < kv.second.size() - 1; ++i) {
        os << kv.second[i] << "|";
      }
      os << kv.second[kv.second.size() - 1] << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, 
                         paracel::list_type<
                         std::pair<paracel::str_type, double> > > & data, 
                         const paracel::str_type & filename = "result_",
                         bool merge = false) {
    std::ofstream os;
    os.open(paracel::todir(output) + filename + std::to_string(worker_comm.get_rank()), std::ofstream::app);
    for(auto & kv : data) {
      os << kv.first + '\t';
      for(size_t i = 0; i < kv.second.size() - 1; ++i) {
        os << kv.second[i].first << ':'
            << std::to_string(kv.second[i].second) << '|';
      }
      os << kv.second[kv.second.size() - 1].first << ":" 
          << kv.second[kv.second.size() - 1].second  << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  template <class T, class F>
  T data_merge(T & data, F & func, paracel::Comm & local_comm, int rank = 0) {
    return local_comm.treereduce(data, func, rank);
  }

  virtual void solve() {}
  //virtual void solve() = 0;

 private:
  class parasrv {

    using l_type = paracel::list_type<paracel::kvclt>;
    using dl_type = paracel::list_type<paracel::dict_type<paracel::str_type, paracel::str_type> >; 

   public:
    parasrv(paracel::str_type hosts_dct_str) {
      // init dct_lst
      dct_lst = paracel::get_hostnames_dict(hosts_dct_str);
      // init srv_sz
      srv_sz = dct_lst.size();
      // init kvm
      for(auto & srv : dct_lst) {
        paracel::kvclt kvc(srv["host"], srv["ports"]);
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

   public:
    dl_type dct_lst;
    int srv_sz = 1;
    l_type kvm;
    paracel::list_type<int> servers;
    paracel::ring<int> *p_ring;
  }; // nested class definition

 private:
  int stale_cache, clock, total_iters;
  int clock_server = 0;
  paracel::Comm worker_comm;
  paracel::str_type output;
  int nworker = 1;
  int rounds = 1;
  int limit_s = 0;
  bool ssp_switch = false;
  parasrv *ps_obj;
  paracel::dict_type<size_t, paracel::str_type> rm;
  paracel::dict_type<size_t, paracel::str_type> cm;
  paracel::dict_type<size_t, int> dm;
  paracel::dict_type<size_t, int> col_dm;
  paracel::dict_type<paracel::str_type, paracel::str_type> keymap;
  paracel::dict_type<paracel::str_type, boost::any> cached_para;
  paracel::update_result update_f;
  int npx = 1, npy = 1;
 private:
  template <class V>
  struct min_heap_cmp {
    inline bool operator() (const std::pair<paracel::str_type, V> & l,
                            const std::pair<paracel::str_type, V> & r) {
      return l.second > r.second;
    }
  };
}; // class paralg

} // namespace paracel

#endif
