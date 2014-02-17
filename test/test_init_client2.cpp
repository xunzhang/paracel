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
  paracel::str_type init_port = paracel::gen_init_port();
  auto ports_tmp = paracel::get_hostnames_string(1, "7773"); 
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
  {
    std::unordered_map<paracel::str_type, int> tmp{std::pair<paracel::str_type, int>(paracel::str_type("x"), 100), std::pair<paracel::str_type, int>(paracel::str_type("y"), 200)};
    kvc.push_multi(tmp);
    std::cout << kvc.contains(paracel::str_type("x")) << std::endl;
    std::cout << kvc.pull<int>(paracel::str_type("x")) << std::endl;
    std::cout << kvc.contains(paracel::str_type("y")) << std::endl;
    std::cout << kvc.pull<int>(paracel::str_type("y")) << std::endl;
  }
  {
    paracel::str_type key2 = "key_3";
    kvc.push(key2, 3.1415);
    std::cout << kvc.pull<double>(key2) << std::endl;
    kvc.update(key2, 4.6362);
    std::chrono::milliseconds dura(1);
    std::this_thread::sleep_for(dura);
    std::cout << kvc.pull<double>(key2) << std::endl;
  }
  {
    paracel::str_type key2 = "key_3";
    kvc.register_bupdate(paracel::str_type("/mfs/user/wuhong/paracel/lib/local.so"), paracel::str_type("local_update"));
    kvc.bupdate(key2, 10.);
    /*
    kvc.register_update(paracel::str_type("/mfs/user/wuhong/paracel/lib/local.so"), paracel::str_type("local_update"));
    kvc.update(key2, 10.);
    std::chrono::milliseconds dura(100);
    std::this_thread::sleep_for(dura);
    */
    std::cout << kvc.pull<double>(key2) << std::endl;
  }
  return 0;
}
