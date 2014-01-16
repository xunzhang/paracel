#include <iostream>
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
  return 0;
}
