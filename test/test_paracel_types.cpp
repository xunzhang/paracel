#include <iostream>
#include <vector>
#include <string>
#include "paracel_types.hpp"
#include <douban/hash.hpp>

int main(int argc, char *argv[])
{
  std::cout << paracel::is_atomic<int>::value << std::endl;
  std::cout << paracel::is_atomic<char>::value << std::endl;
  std::cout << paracel::is_atomic<std::vector<int>>::value << std::endl;
  std::cout << paracel::is_seqic<std::vector<int>>::value << std::endl;
  std::cout << paracel::is_seqic<std::vector<double>>::value << std::endl;

  paracel::hash_type<std::string> hfunc;
  std::cout << hfunc("abcd") << std::endl;
  //paracel::hash_type< std::vector<bool> > hfunc2;
  //std::vector<bool> tmp{true, false, false};
  //std::cout << hfunc2(tmp) << std::endl;

  std::vector<int> tmp2{1, 2, 3};
  paracel::hash_type< std::vector<int> > csfunc;
  std::cout << csfunc(tmp2) << std::endl;

  std::vector<char> tmp3{'a', 'b', 'c'};
  paracel::hash_type< std::vector<char> > csfunc2;
  std::cout << csfunc2(tmp3) << std::endl;

  std::cout << "---" << std::endl;

  paracel::bag_type<int> bg;
  bg.put(1); bg.put(3); bg.put(5);
  auto bg_lambda = [] (int item) { 
    std::cout << item << std::endl;
  };
  bg.traverse(bg_lambda);
  auto bg_cp = bg.get();
  std::cout << bg_cp.size() << std::endl;
  bg_cp.traverse(bg_lambda);

  paracel::bag_type<int> bg2(bg);
  bg2.traverse(bg_lambda);

  for(auto & i : bg2) {
    std::cout << i << std::endl;
  }
  return 0;
}
