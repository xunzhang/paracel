// mpic++ -std=c++11 test_partition.cpp /usr/lib/libboost_context.so -I../include
#include <iostream>
#include <vector>
#include <deque>
#include <list>

#include <boost/bind.hpp>

#include "paracel_types.hpp"
#include "load/partition.hpp"

template <class T = int>
void xrange_impl(paracel::coroutine<int>::caller_type & yield, int limit) {
  for(int i = 0; i < limit; ++i) {
    yield(i);
  }
}

int main(int argc, char *argv[]) {

  {
    paracel::coroutine<int> xrange(boost::bind(xrange_impl, _1, 10000));
    int sum = 0;
    while(xrange) {
      auto tmp = xrange.get();
      std::cout << tmp << std::endl;
      sum += tmp;
      xrange();
    }
    //std::vector<decltype(xrange)> cc;
    //cc.emplace_back(std::move(xrange));
    std::deque<decltype(xrange)> bb;
    bb.emplace_back(std::move(xrange));
    //bb.push_back(std::move(xrange));
    //auto ss = bb[0];
  }

  { // test file_load_lines_impl
    paracel::coroutine<paracel::str_type> file_load_lines(boost::bind(paracel::file_load_lines_impl, _1, "a.txt", 0, 67));
    while(file_load_lines) {
      auto tmp = file_load_lines.get();
      std::cout << tmp << std::endl;
      file_load_lines();
    }
  }

  { // test file_partition
    auto loads = paracel::file_partition("a.txt", 2);
    for(auto & line_iterable : loads) {
      while(line_iterable) {
        auto line = line_iterable.get();
	std::cout << line << std::endl;
	line_iterable();
      }
      std::cout << "=======================" << std::endl;
    } // end of for
  } // end of block

  { // test files_load_lines_impl
    std::cout << std::endl;
    std::cout << std::endl;
    paracel::list_type<paracel::str_type> name_lst{"a.txt"};
    paracel::list_type<int> dis{0};
    paracel::coroutine<paracel::str_type> files_load_lines(boost::bind(paracel::files_load_lines_impl, _1, name_lst, dis, 0, 67));
    while(files_load_lines) {
      auto tmp = files_load_lines.get();
      std::cout << tmp << std::endl;
      files_load_lines();
    }
  }

  { // test files_partition
    std::cout << "----------------" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    paracel::list_type<paracel::str_type> name_lst{"a.txt"};
    auto loads = paracel::files_partition(name_lst, 2);
    for(auto & line_iterable : loads) {
      while(line_iterable) {
        auto line = line_iterable.get();
	std::cout << line << std::endl;
	line_iterable();
      }
      std::cout << "----------------" << std::endl;
    }
  }

  { // test files_partition another
    std::cout << "----------------" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    paracel::list_type<paracel::str_type> name_lst{"a.txt", "b.txt"};
    auto loads = paracel::files_partition(name_lst, 2);
    for(auto & line_iterable : loads) {
      while(line_iterable) {
        auto line = line_iterable.get();
	std::cout << line << std::endl;
	line_iterable();
      }
      std::cout << "----------------" << std::endl;
    }
  }

  return 0;
}
