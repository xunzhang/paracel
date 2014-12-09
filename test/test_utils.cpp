#include <iostream>
#include <string>
#include <vector>
#include "utils.hpp"

int main(int agrc, char *argv[])
{
  paracel::str_type s = "beater7:61400,13068,65342,36347PARACELbeater7:18722,54347,56295,16430";
  auto r = paracel::get_hostnames_dict(s);
  for(auto & t : r) {
    for(auto & v : t) {
      std::cout << v.first << " : " << v.second << std::endl;
    } 
  }

  paracel::str_type s2 = "39352476";
  auto v = paracel::str_split(s2, '|');
  std::cout << v.size() << " | " << v[0] << std::endl;

  std::string fn = "unit_test.data";
  auto lambda = [] (std::string s) { return s + "STOP"; };
  auto lines = paracel::tail<std::string>(fn, 3, lambda);
  for(auto & line : lines) {
    std::cout << line << std::endl;
  }
  return 0;
}
