#include <iostream>
#include <utility>
#include <tuple>
#include "paracel_types.hpp"
#include "graph.hpp"

int main(int argc, char *argv[])
{
  { // test for bigraph
    paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
    tpls.emplace_back(std::make_tuple(0, 0, 3.));
    tpls.emplace_back(std::make_tuple(0, 2, 5.));
    tpls.emplace_back(std::make_tuple(1, 0, 4.));
    tpls.emplace_back(std::make_tuple(1, 1, 3.));
    tpls.emplace_back(std::make_tuple(1, 2, 1.));
    tpls.emplace_back(std::make_tuple(2, 0, 2.));
    tpls.emplace_back(std::make_tuple(2, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 1, 3.));
    tpls.emplace_back(std::make_tuple(3, 3, 1.));
    paracel::bigraph<size_t> grp(tpls);
    paracel::bigraph<size_t> grp2;
    std::cout << "duck" << grp2.v() << std::endl;
    grp2.construct_from_triples(tpls);
    // traverse
    for(int i = 0; i < grp.v(); ++i) {
      std::cout << i << " : ";
      for(auto & ed : grp.adjacent(i)) {
        std::cout << ed.first;
      }
      std::cout << std::endl;
    }
    for(auto & w : grp.adjacent(0)) {
      std::cout << w.first << std::endl;
    }
    auto f = [] (size_t a, size_t b, double c) {
      std::cout << a << " | " << b << " | " << c << std::endl;
    };
    grp.traverse(f);
    std::cout << "!!!" << std::endl;
    grp.traverse(1, f);
    std::cout << "!!!" << std::endl;
    grp2.traverse(f);
    std::cout << "!!!" << std::endl;
    grp2.traverse(1, f);
    std::cout << "!!!" << std::endl;
    paracel::list_type<std::tuple<size_t, size_t, double> > local_tpls;
    grp.dump2triples(local_tpls);
    std::cout << "test" << std::get<0>(local_tpls[0]) << " | " << std::get<1>(local_tpls[0]) << " | " << std::get<2>(local_tpls[0]) << std::endl;
  }
  { // test for undirected graph
    paracel::undirected_graph grp(13);
    grp.add_edge(0, 1);
    grp.add_edge(0, 2);
    grp.add_edge(0, 5);
    grp.add_edge(0, 6);
    grp.add_edge(3, 4);
    grp.add_edge(3, 5);
    grp.add_edge(4, 5);
    grp.add_edge(4, 6);
    //grp.add_edge(7, 8);
    grp.add_edge(9, 10);
    grp.add_edge(9, 11);
    grp.add_edge(9, 12);
    grp.add_edge(11, 12);
    std::cout << grp.v() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.e() << std::endl;
    std::cout << "----------------------" << std::endl;
    for(auto & w : grp.adjacent(5)) {
      std::cout << w.first << std::endl;
    }
    // traverse
    for(int i = 0; i < grp.v(); ++i) {
      std::cout << i << " : ";
      for(auto & ed : grp.adjacent(i)) {
        std::cout << ed.first;
      }
      std::cout << std::endl;
    }
    std::cout << "----------------------" << std::endl;
    std::cout << grp.avg_degree() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.max_degree() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.selfloops() << std::endl;
    std::cout << "======================" << std::endl;
  }
  { // test for undirected graph
    paracel::list_type<std::pair<size_t, size_t> > edges;
    edges.emplace_back(std::make_pair(0, 1));
    edges.emplace_back(std::make_pair(0, 2));
    edges.emplace_back(std::make_pair(0, 5));
    edges.emplace_back(std::make_pair(0, 6));
    edges.emplace_back(std::make_pair(3, 4));
    edges.emplace_back(std::make_pair(3, 5));
    edges.emplace_back(std::make_pair(4, 5));
    edges.emplace_back(std::make_pair(4, 6));
    //edges.emplace_back(std::make_pair(7, 8));
    edges.emplace_back(std::make_pair(9, 10));
    edges.emplace_back(std::make_pair(9, 11));
    edges.emplace_back(std::make_pair(9, 12));
    edges.emplace_back(std::make_pair(11, 12));
    paracel::undirected_graph grp(edges);
    std::cout << grp.v() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.e() << std::endl;
    std::cout << "----------------------" << std::endl;
    for(auto & w : grp.adjacent(5)) {
      std::cout << w.first << std::endl;
    }
    for(int i = 0; i < grp.v(); ++i) {
      std::cout << i << " : ";
      for(auto & ed : grp.adjacent(i)) {
        std::cout << ed.first;
      }
      std::cout << std::endl;
    }
    std::cout << "----------------------" << std::endl;
    std::cout << grp.avg_degree() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.max_degree() << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << grp.selfloops() << std::endl;
    std::cout << "----------------------" << std::endl;
  }
  {
    paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
    tpls.emplace_back(std::make_tuple(4, 2, 1.));
    tpls.emplace_back(std::make_tuple(2, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 2, 1.));
    tpls.emplace_back(std::make_tuple(6, 0, 1.));
    tpls.emplace_back(std::make_tuple(0, 1, 1.));
    tpls.emplace_back(std::make_tuple(2, 0, 1.));
    tpls.emplace_back(std::make_tuple(11, 12, 1.));
    tpls.emplace_back(std::make_tuple(12, 9, 1.));
    tpls.emplace_back(std::make_tuple(9, 10, 1.));
    tpls.emplace_back(std::make_tuple(9, 11, 1.));
    tpls.emplace_back(std::make_tuple(8, 9, 1.));
    tpls.emplace_back(std::make_tuple(10, 12, 1.));
    tpls.emplace_back(std::make_tuple(11, 4, 1.));
    tpls.emplace_back(std::make_tuple(4, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 5, 1.));
    tpls.emplace_back(std::make_tuple(6, 8, 1.));
    tpls.emplace_back(std::make_tuple(8, 6, 1.));
    tpls.emplace_back(std::make_tuple(5, 4, 1.));
    tpls.emplace_back(std::make_tuple(0, 5, 1.));
    tpls.emplace_back(std::make_tuple(6, 4, 1.));
    tpls.emplace_back(std::make_tuple(6, 9, 1.));
    tpls.emplace_back(std::make_tuple(7, 6, 1.));
    paracel::bigraph<size_t> grp(tpls);
    auto lambda = [] (size_t a) {
      std::cout << a << std::endl;
    };
    std::cout << "~" << std::endl;
    paracel::DFS<paracel::bigraph<size_t>, size_t, decltype(lambda)> dfs_o1(grp, 0, lambda);
    std::cout << "~" << std::endl;
    paracel::DFS<paracel::bigraph<size_t>, size_t, decltype(lambda)> dfs_o2(grp, 1, lambda);
    std::cout << "~" << std::endl;
    paracel::DFS<paracel::bigraph<size_t>, size_t, decltype(lambda)> dfs_o3(grp, 6, lambda);
    std::cout << "~" << std::endl;
    paracel::list_type<std::pair<size_t, size_t> > edges;
    edges.emplace_back(std::make_pair(0, 5));
    edges.emplace_back(std::make_pair(4, 3));
    edges.emplace_back(std::make_pair(0, 1));
    edges.emplace_back(std::make_pair(9, 12));
    edges.emplace_back(std::make_pair(6, 4));
    edges.emplace_back(std::make_pair(5, 4));
    edges.emplace_back(std::make_pair(0, 2));
    edges.emplace_back(std::make_pair(11, 12));
    edges.emplace_back(std::make_pair(9, 10));
    edges.emplace_back(std::make_pair(0, 6));
    edges.emplace_back(std::make_pair(7, 8));
    edges.emplace_back(std::make_pair(9, 11));
    edges.emplace_back(std::make_pair(5, 3));
    paracel::undirected_graph grp2(edges);
    std::cout << "~" << std::endl;
    paracel::DFS<paracel::undirected_graph, size_t, decltype(lambda)> dfs_o4(grp2, 0, lambda);
    std::cout << "~" << std::endl;
  }
  {
    paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
    tpls.emplace_back(std::make_tuple(5, 0, 1.));
    tpls.emplace_back(std::make_tuple(2, 4, 1.));
    tpls.emplace_back(std::make_tuple(3, 2, 1.));
    tpls.emplace_back(std::make_tuple(1, 2, 1.));
    tpls.emplace_back(std::make_tuple(0, 1, 1.));
    tpls.emplace_back(std::make_tuple(4, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 5, 1.));
    tpls.emplace_back(std::make_tuple(0, 2, 1.));
    paracel::bigraph<size_t> grp(tpls);
    auto lambda = [] (size_t a) {
      std::cout << a << std::endl;
    };
    std::cout << "~" << std::endl;
    paracel::BFS<paracel::bigraph<size_t>, size_t, decltype(lambda)> bfs_o1(grp, 0, lambda);
    std::cout << "~" << std::endl;
    std::cout << "edgeTo 0 " << bfs_o1.edgeTo(0) << std::endl;
    std::cout << "edgeTo 1 " << bfs_o1.edgeTo(1) << std::endl;
    std::cout << "edgeTo 2 " << bfs_o1.edgeTo(2) << std::endl;
    std::cout << "edgeTo 3 " << bfs_o1.edgeTo(3) << std::endl;
    std::cout << "edgeTo 4 " << bfs_o1.edgeTo(4) << std::endl;
    std::cout << "edgeTo 5 " << bfs_o1.edgeTo(5) << std::endl;
    std::cout << "dist_to 0 " << bfs_o1.dist(0) << std::endl;
    std::cout << "dist_to 1 " << bfs_o1.dist(1) << std::endl;
    std::cout << "dist_to 2 " << bfs_o1.dist(2) << std::endl;
    std::cout << "dist_to 3 " << bfs_o1.dist(3) << std::endl;
    std::cout << "dist_to 4 " << bfs_o1.dist(4) << std::endl;
    std::cout << "dist_to 5 " << bfs_o1.dist(5) << std::endl;

    paracel::list_type<std::pair<size_t, size_t> > edges;
    edges.emplace_back(std::make_pair(0, 5));
    edges.emplace_back(std::make_pair(2, 4));
    edges.emplace_back(std::make_pair(2, 3));
    edges.emplace_back(std::make_pair(1, 2));
    edges.emplace_back(std::make_pair(0, 1));
    edges.emplace_back(std::make_pair(3, 4));
    edges.emplace_back(std::make_pair(3, 5));
    edges.emplace_back(std::make_pair(0, 2));
    paracel::undirected_graph grp2(edges);
    paracel::BFS<paracel::undirected_graph, size_t, decltype(lambda)> bfs_o2(grp2, 0, lambda);
    std::cout << "edgeTo 0 " << bfs_o2.edgeTo(0) << std::endl;
    std::cout << "edgeTo 1 " << bfs_o2.edgeTo(1) << std::endl;
    std::cout << "edgeTo 2 " << bfs_o2.edgeTo(2) << std::endl;
    std::cout << "edgeTo 3 " << bfs_o2.edgeTo(3) << std::endl;
    std::cout << "edgeTo 4 " << bfs_o2.edgeTo(4) << std::endl;
    std::cout << "edgeTo 5 " << bfs_o2.edgeTo(5) << std::endl;
    std::cout << "dist_to 0 " << bfs_o2.dist(0) << std::endl;
    std::cout << "dist_to 1 " << bfs_o2.dist(1) << std::endl;
    std::cout << "dist_to 2 " << bfs_o2.dist(2) << std::endl;
    std::cout << "dist_to 3 " << bfs_o2.dist(3) << std::endl;
    std::cout << "dist_to 4 " << bfs_o2.dist(4) << std::endl;
    std::cout << "dist_to 5 " << bfs_o2.dist(5) << std::endl;
  }
  {
    paracel::list_type<std::pair<size_t, size_t> > edges;
    edges.emplace_back(std::make_pair(0, 5));
    edges.emplace_back(std::make_pair(4, 3));
    edges.emplace_back(std::make_pair(0, 1));
    edges.emplace_back(std::make_pair(9, 12));
    edges.emplace_back(std::make_pair(6, 4));
    edges.emplace_back(std::make_pair(5, 4));
    edges.emplace_back(std::make_pair(0, 2));
    edges.emplace_back(std::make_pair(11, 12));
    edges.emplace_back(std::make_pair(9, 10));
    edges.emplace_back(std::make_pair(0, 6));
    edges.emplace_back(std::make_pair(7, 8));
    edges.emplace_back(std::make_pair(9, 11));
    edges.emplace_back(std::make_pair(5, 3));
    paracel::undirected_graph grp(edges);
    paracel::connected_components<paracel::undirected_graph, size_t> cc_o(grp);
    std::cout << "cnt of cc: " << cc_o.cnt() << std::endl;
    std::cout << "identifier of 0: " << cc_o.id(0) << std::endl;
    std::cout << "identifier of 1: " << cc_o.id(1) << std::endl;
    std::cout << "identifier of 2: " << cc_o.id(2) << std::endl;
    std::cout << "identifier of 3: " << cc_o.id(3) << std::endl;
    std::cout << "identifier of 4: " << cc_o.id(4) << std::endl;
    std::cout << "identifier of 5: " << cc_o.id(5) << std::endl;
    std::cout << "identifier of 6: " << cc_o.id(6) << std::endl;
    std::cout << "identifier of 7: " << cc_o.id(7) << std::endl;
    std::cout << "identifier of 8: " << cc_o.id(8) << std::endl;
    std::cout << "identifier of 9: " << cc_o.id(9) << std::endl;
    std::cout << "identifier of 10: " << cc_o.id(10) << std::endl;
    std::cout << "identifier of 11: " << cc_o.id(11) << std::endl;
    std::cout << "identifier of 12: " << cc_o.id(12) << std::endl;
  }
  return 0;
}
