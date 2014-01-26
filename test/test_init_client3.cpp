#include <iostream>
#include <unordered_map>

#include "utils.hpp"
#include "paracel_types.hpp"
#include "client.hpp"
#include "packer.hpp"

int main(int argc, char *argv[])
{
  auto ports_tmp = paracel::get_hostnames_string(1); 
  paracel::str_type ports(ports_tmp.begin() + 8, ports_tmp.end());
  paracel::kvclt kvc("beater7", ports);
  {
    paracel::str_type key0 = "key_0";
    paracel::str_type key1 = "key_1";
    kvc.push_int(key0, 1);
    kvc.push_int(key1, 2);
  }
  {
    auto d = kvc.pullall<int>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  {
    paracel::str_type key0 = "key_0";
    kvc.incr_int(key0, 9);
    auto d = kvc.pullall<int>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  return 0;
}
