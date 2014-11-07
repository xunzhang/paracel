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
#ifndef FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP
#define FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP

#include <tuple>
#include <utility>
#include <queue>
#include <algorithm>
#include <msgpack.hpp>
#include "paracel_types.hpp"

namespace paracel {

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
  
  paracel::list_type<std::pair<size_t, double> > adjacent(size_t v) { return adj[v]; }
  
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
  paracel::list_type<paracel::list_type<std::pair<size_t, double> > > adj;
 
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj);
};

template <class T = paracel::str_type>
class bigraph {

public:
  bigraph() {}

  bigraph(paracel::dict_type<T, paracel::dict_type<T, double> > edge_info) {
    construct_from_dict(edge_info);
  }
  
  bigraph(paracel::list_type<std::tuple<T, T> > tpls) {
    construct_from_tuples(tpls);
  }
  
  bigraph(paracel::list_type<std::tuple<T, T, double> > tpls) {
    construct_from_triples(tpls);
  }

  void construct_from_dict(const paracel::dict_type<T, paracel::dict_type<T, double> > & edge_info) {
    for(auto & edge : edge_info) {
      for(auto & kv : edge.second)
      add_edge(edge.first, kv.first, kv.second);
    }
  }

  void construct_from_tuples(const paracel::list_type<std::tuple<T, T> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl));
    }
  }
  
  void construct_from_triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }
  
  void add_edge(const T & v, const T & w) {
    add_egde(v, w, 1.);
  }

  void add_edge(const T & v, const T & w, double wgt) {  
    adj[v][w] = wgt;
    reverse_adj[w][v] = wgt;
    e_sz += 1; // suppose no repeat
    v_sz = std::max(adj.size(), reverse_adj.size());
  }
  
  paracel::dict_type<T, paracel::dict_type<T, double> > get_data() {
    return adj;
  }

  template <class F>
  void traverse(F & func) {  
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        func(v.first, kv.first, kv.second);
      }
    }
  }

  template <class F>
  void traverse(const T & v, F & func) {
    for(auto & kv : adj[v]) {
      func(v, kv.first, kv.second);
    }
  }

  template <class F>
  void traverse_by_vertex(F & func) {
    for(auto & v : adj) {
      func(v);
    }
  }

  std::vector<T> vertex_bag() {
    std::vector<T> r;
    for(auto & v : adj) {
      r.push_back(v.first);
    }
  }
  
  void dump2triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    tpls.resize(0);
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        tpls.push_back(std::make_tuple(v.first, kv.first, kv.second));
      }
    }
  }

  void dump2dict(paracel::dict_type<T, paracel::dict_type<T, double> > & dict) {
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        dict[v.first][kv.first] = kv.second;
      }
    }
  }
  
  bigraph reverse() {
    std::swap(adj, reverse_adj);
  }

  inline size_t v() { 
    return v_sz; 
  }
  
  inline size_t e() { 
    return e_sz; 
  }
  
  paracel::dict_type<T, double> 
  adjacent(const T & v) {
    return adj[v];
  }
  
  paracel::dict_type<T, double>
  reverse_adjacent(const T & v) {
    return reverse_adj[v];
  }

  inline size_t outdegree(const T & v) { 
    return adj[v].size(); 
  }

  inline size_t indegree(const T & vertex) {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(vertex == kv.first) {
	  cnt += 1;
	}
      }
    }
    return cnt;
  }
  
  inline double avg_degree() { 
    return e_sz / v_sz; 
  }
  
  inline int selfloops() {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(v.first == kv.first) {
	  cnt += 1;
	}
      }
    }
    return cnt;
  }

 private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  paracel::dict_type<T, paracel::dict_type<T, double> > adj;
  paracel::dict_type<T, paracel::dict_type<T, double> > reverse_adj;
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj, reverse_adj);
};

template <class G, class T, class F>
class DFS {
 public:
  DFS(G graph, T src, F lambda) {
    dfs(graph, src, lambda);
  }
  inline bool visited(const T & v) { return marked.count(v); }
 private:
  void dfs(G & graph, const T & v, F & func) {
    marked[v] = true;
    func(v);
    for(auto & kv : graph.adjacent(v)) {
      auto w = kv.first;
      if(!marked.count(w)) {
        dfs(graph, w, func);
      }
    }
  } // dfs
 private:
  paracel::dict_type<T, bool> marked;
}; // class DFS

// specialization DFS for undirected_graph
template <class F>
class DFS<paracel::undirected_graph, size_t, F> {
 public:
  DFS(paracel::undirected_graph graph, size_t src, F lambda) {
    dfs(graph, src, lambda);
  }
  inline bool visited(size_t v) { return marked.count(v); }
 private:
  void dfs(paracel::undirected_graph graph, size_t v, F & func) {
    marked[v] = true;
    func(v);
    for(auto & pr : graph.adjacent(v)) {
      if(!marked.count(pr.first)) {
        dfs(graph, pr.first, func);
      }
    }
  } // dfs
 private:
  paracel::dict_type<size_t, bool> marked;
}; // specialization class DFS

template <class G, class T, class F>
class BFS {
 public:
  BFS(G graph, T src, F func) {
    bfs(graph, src, func);
  }
  inline bool visited(const T & v) { return marked.count(v); }
  inline T edgeTo(const T & v) { return edge_to[v]; }
  inline int dist(const T & v) { return dist_to[v]; }
 private:
  void bfs(G & grp, const T & v, F & func) {
    std::queue<T> q;
    q.push(v);
    marked[v] = true;
    func(v);
    dist_to[v] = 0;
    while(!q.empty()) {
      T node = q.front();
      q.pop();
      bool has_unmarked = false;
      for(auto & kv : grp.adjacent(node)) {
        auto w = kv.first;
        if(!marked.count(w)) {
          has_unmarked = true;
          q.push(w);
          marked[w] = true;
          func(w);
          edge_to[w] = node;
          dist_to[w] = dist_to[node] + 1;
        }
      }
    }
  }
 private:
  paracel::dict_type<T, bool> marked;
  paracel::dict_type<T, T> edge_to;
  paracel::dict_type<T, int> dist_to;
}; // class BFS

} // namespace paracel

#endif
