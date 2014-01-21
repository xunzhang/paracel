#include <iostream>

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
    kvc.push(key0, 1);
    kvc.push(key1, 2);
  }
  {
    auto d = kvc.pullall<int>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  {
    paracel::dict_type<paracel::str_type, paracel::str_type> tmp_d;
    auto res = kvc.pullall(tmp_d);
    std::cout << res << std::endl;
    paracel::packer<int> pk;
    for(auto & kv : tmp_d) {
      std::cout << kv.first << ":" << pk.unpack(kv.second) << std::endl;
    }
  } 
  {
    paracel::str_type key2 = "key_2";
    kvc.push(key2, paracel::str_type("sc"));
    paracel::dict_type<paracel::str_type, paracel::str_type> tmp_d;
    auto res = kvc.pullall(tmp_d);
    std::cout << res << std::endl;
    paracel::packer<int> pk;
    paracel::packer<paracel::str_type> pk2;
    for(auto & kv : tmp_d) {
      auto tmp = paracel::str_split(kv.first, '_');
      if(tmp[1] == "2") {
        std::cout << kv.first << ":" << pk2.unpack(kv.second) << std::endl;
      } else {
        std::cout << kv.first << ":" << pk.unpack(kv.second) << std::endl;
      }
    }
  } 
  /*
  {
    std::unordered_map<paracel::str_type, int> tmp{std::pair<paracel::str_type, int>(paracel::str_type("x"), 100), std::pair<paracel::str_type, int>(paracel::str_type("y"), 200)};
    kvc.push_multi(tmp);
    std::cout << kvc.contains(paracel::str_type("x")) << std::endl;
    std::cout << kvc.pull<int>(paracel::str_type("x")) << std::endl;
  }
  */
  return 0;
}
