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
    paracel::bigraph grp(tpls);
    paracel::bigraph grp2;
    std::cout << "duckduckgo" << grp2.v() << std::endl;
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
    auto f = [] (std::tuple<size_t, size_t, double> tpl) {
      std::cout << std::get<0>(tpl) << " | " << std::get<1>(tpl) << " | " << std::get<2>(tpl) << std::endl;
    };
    grp.traverse(f);
    grp.traverse(1, f);
    grp2.traverse(f);
    grp2.traverse(1, f);
    paracel::list_type<std::tuple<size_t, size_t, double> > local_tpls;
    grp.dump_triples(local_tpls);
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
  return 0;
}
