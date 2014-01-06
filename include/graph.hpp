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
#ifndef FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP
#define FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP

#include <tuple>
#include <utility>
#include <algorithm>
#include "paracel_types.hpp"

namespace paracel {

template <class T>
using bag_type = paracel::list_type<T>;

// (size_t, size_t, double) type undirected graph
class undirected_graph {
public:
  
  undirected_graph(size_t v_sz) {
    this->v_sz = v_sz;
    adj.resize(v_sz);
  }
  
  undirected_graph(paracel::list_type<std::pair<size_t, size_t> > & edges) {
    for(auto & edge : edges) {
      if(edge.first > v_sz) v_sz = edge.first;
      if(edge.second > v_sz) v_sz = edge.second;
    }
    v_sz += 1;
    adj.resize(v_sz);
    for(auto & edge : edges) {
      add_edge(edge.first, edge.second);
    }
  }

  undirected_graph(paracel::list_type<std::tuple<size_t, size_t, double> > & tpls) {
    for(auto & tpl : tpls) {
      if(std::get<0>(tpl) > v_sz) v_sz = std::get<0>(tpl);
      if(std::get<1>(tpl) > v_sz) v_sz = std::get<1>(tpl);
    }
    v_sz += 1;
    adj.resize(v_sz);
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }

  void add_edge(size_t v, size_t w) {
    adj[v].emplace_back(std::make_pair(w, 1.));
    adj[w].emplace_back(std::make_pair(v, 1.));
    e_sz += 1;
  }

  void add_edge(size_t v, size_t w, double wgt) { 
    adj[v].emplace_back(std::make_pair(w, wgt));
    adj[w].emplace_back(std::make_pair(v, wgt));
    e_sz += 1;
  }

  inline size_t v() { return v_sz; }
  
  inline size_t e() { return e_sz; }
  
  bag_type<std::pair<size_t, double> > adjacent(size_t v) { return adj[v]; }
  
  inline size_t degree(size_t v) { return adj[v].size(); }
  
  inline double avg_degree() { return 2. * e_sz / v_sz; }
  
  inline size_t max_degree() {
    size_t max = 0;
    for(size_t v = 0; v < v_sz; ++v) {
      if(degree(v) > max) {
        max = degree(v);
      }
    }
    return max;
  }
  
  inline int selfloops() {
    int cnt = 0;
    for(size_t v = 0; v < v_sz; ++v) {
      for(auto & ww : adj[v]) {
        if(v == std::get<0>(ww)) { cnt ++; }
      }
    }
    return cnt / 2;
  }

private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  bag_type<bag_type<std::pair<size_t, double> > > adj;
};

// (size_t, size_t, double) type directed graph
class bigraph {
public:
  
  bigraph(size_t v_sz) {
    this->v_sz = v_sz;
    adj.resize(v_sz);
  }
  
  bigraph(paracel::list_type<std::pair<size_t, size_t> > & edges) {
    for(auto & edge : edges) {
      if(edge.first > v_sz) v_sz = edge.first;
      if(edge.second > v_sz) v_sz = edge.second;
    }
    v_sz += 1;
    adj.resize(v_sz);
    for(auto & edge : edges) {
      add_edge(edge.first, edge.second);
    }
  }

  bigraph(paracel::list_type<std::tuple<size_t, size_t, double> > & tpls) {
    for(auto & tpl : tpls) {
      if(std::get<0>(tpl) > v_sz) v_sz = std::get<0>(tpl);
      if(std::get<1>(tpl) > v_sz) v_sz = std::get<1>(tpl);
    }
    v_sz += 1;
    adj.resize(v_sz);
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }

  void add_edge(size_t v, size_t w) {
    adj[v].emplace_back(std::make_pair(w, 1.));
    reverse_adj[w].emplace_back(std::make_pair(v, 1.));
    e_sz += 1;
  }

  void add_edge(size_t v, size_t w, double wgt) {  
    adj[v].emplace_back(std::make_pair(w, wgt));
    reverse_adj[w].emplace_back(std::make_pair(v, wgt));
    e_sz += 1;
  }
  
  bigraph reverse() {
  }

  inline size_t v() { return v_sz; }
  
  inline size_t e() { return e_sz; }
  
  bag_type<std::pair<size_t, double> > adjacent(size_t v) { return adj[v]; }
  
  
  inline size_t outdegree(size_t v) { return adj[v].size(); }

  inline size_t indegree(size_t v) {}
  
  inline double avg_degree() { return e_sz / v_sz; }
  
  inline int selfloops() {
    int cnt = 0;
    for(size_t v = 0; v < v_sz; ++v) {
      for(auto & ww : adj[v]) {
        if(v == std::get<0>(ww)) { cnt ++; }
      }
    }
    return cnt / 2;
  }

private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  bag_type<bag_type<std::pair<size_t, double> > > adj;
  bag_type<bag_type<std::pair<size_t, double> > > reverse_adj;
};

} // namespace paracel

#endif
