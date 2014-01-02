#include <iostream>
#include <utility>
#include "paracel_types.hpp"
#include "graph.hpp"

int main(int argc, char *argv[])
{
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
    grp.add_edge(7, 8);
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
    edges.emplace_back(std::make_pair(7, 8));
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
