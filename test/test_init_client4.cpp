#include <iostream>
#include <unordered_map>
#include <chrono>
#include <thread>

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
    paracel::str_type key2 = "key_2";
    paracel::str_type key3 = "key_3";
    kvc.push(key0, 1.);
    kvc.push(key1, 2.);
    kvc.push(key2, 3.);
    kvc.push(key3, 4.);
  }
  {
    auto d = kvc.pullall<double>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  {
    paracel::str_type key2 = "key_3";
    kvc.register_update(paracel::str_type("/mfs/user/wuhong/paracel/lib/local.so"), paracel::str_type("local_update"));
    kvc.update(key2, 10.);
    std::chrono::milliseconds dura(10);
    std::this_thread::sleep_for(dura);
    std::cout << "update pull: " << kvc.pull<double>(key2) << std::endl;
  }
  {
    paracel::str_type key2 = "key_4";
    paracel::str_type key3 = "key_5";
    paracel::str_type key4 = "key_6";
    kvc.push(key2, 11.);
    kvc.push(key3, 12.);
    kvc.push(key4, 13.);
  }
  {
    auto d = kvc.pullall<double>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  {
    std::cout << "----" << std::endl;
    kvc.register_pullall_special(paracel::str_type("/mfs/user/wuhong/paracel/lib/local.so"), paracel::str_type("local_filter"));
    auto d = kvc.pullall_special<double>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  {
    std::cout << "----" << std::endl;
    kvc.register_remove_special(paracel::str_type("/mfs/user/wuhong/paracel/lib/local.so"), paracel::str_type("local_filter_remove"));
    kvc.remove_special();
    auto d = kvc.pullall<double>();
    for(auto & k : d) {
      std::cout << k.first << ":" << k.second << std::endl;
    }
  }
  return 0;
}
