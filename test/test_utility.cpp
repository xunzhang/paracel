#include "utils/ext_utility.hpp"
#include <iostream>
#include <vector>
#include <string>
int main(int agrc, char *argv[])
{
  {
    auto result = paracel::expand("/home/xunzhang/xunzhang/Proj/paracel/test/test_utility.cpp");
    for(auto & item : result)
      std::cout << item << std::endl;
    std::cout << "---------------" << std::endl;
  }
  {
    auto result = paracel::expand("/mfs/alg/tmp/jasontmp/user_music_factor_model/user_track_rating_for_training/00*");
    for(auto & item : result) {
      std::cout << item << std::endl;
    }
    std::cout << "---------------" << std::endl;
  }
  {
    auto result = paracel::expand("/home/xunzhang/xunzhang/Proj/paracel/test/");
    for(auto & item : result)
      std::cout << item << std::endl;
    std::cout << "---------------" << std::endl;
  }
  {
    auto result = paracel::expand("/home/xunzhang/xunzhang/Proj/paracel/test");
    for(auto & item : result)
      std::cout << item << std::endl;
    std::cout << "---------------" << std::endl;
  }
  {
    auto result = paracel::expand("/home/xunzhang/xunzhang/Proj/paracel/test/*.txt");
    for(auto & item : result)
      std::cout << item << std::endl;
    std::cout << "---------------" << std::endl;
  }
  {
    std::vector<std::string> tmp{"/home/xunzhang/xunzhang/Proj/paracel/test/*.txt", 
    				"/home/xunzhang/xunzhang/Proj/paracel/include/utils/", 
				"/home/xunzhang/xunzhang/Proj/paracel/include/kv.hpp"};
    auto result = paracel::expand(tmp);
    for(auto & item : result)
      std::cout << item << std::endl;
    std::cout << "---------------" << std::endl;
  }
  {
    std::vector<std::string> lst = paracel::paracel_glob("/mfs/alg/tmp/jasontmp/user_music_factor_model/artist_factor/a9");
    for(auto & fn : lst) {
      std::cout << fn << std::endl;
    }
  }
  return 0;
}
