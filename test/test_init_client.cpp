#include <iostream>
#include <tr1/unordered_map>

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
    paracel::str_type key = "test_key";
    auto r = kvc.push(key, 2);
    std::cout << r << std::endl;
    auto r2 = kvc.pull<int>(key);
    std::cout << r2 << std::endl;
  }
  
  {
    paracel::str_type key = "test_key1";
    auto r = kvc.push(key, paracel::str_type("sa"));
    std::cout << r << std::endl;
    auto r2 = kvc.pull<paracel::str_type>(key);
    std::cout << r2 << std::endl;
  }

  {
    paracel::str_type key2 = "test_key2";
    paracel::list_type<double> val2 = {1., 2., 0., 3., 4.};
    auto r_new = kvc.push(key2, val2);
    std::cout << r_new << std::endl;
    auto r2_new = kvc.pull<paracel::list_type<double> >(key2);
    for(auto & v : r2_new) {
      std::cout << v << std::endl;
    }
  }

  {
    paracel::str_type key = "test_key3";
    paracel::str_type key2 = "test_key";
    std::cout << kvc.contains(key) << std::endl;
    std::cout << kvc.contains(key2) << std::endl;
  }
/*
  {
    std::tr1::unordered_map<paracel::str_type, int> tmp{std::pair<paracel::str_type, int>(paracel::str_type("x"), 100), std::pair<paracel::str_type, int>(paracel::str_type("y"), 200)};
    kvc.push_multi(tmp);
    std::cout << kvc.contains(paracel::str_type("x")) << std::endl;
    std::cout << kvc.pull<int>(paracel::str_type("x")) << std::endl;
  }
*/
  {
    paracel::str_type key = "test_keyn";
    paracel::str_type key2 = "test_key";
    auto r = kvc.push(key, 7);
    std::cout << r << std::endl;
    int r2;
    kvc.pull(key, r2);
    std::cout << r2 << std::endl;
    std::cout << "contain " << kvc.contains(key) << std::endl;
    kvc.remove(key);
    int tt = 0;
    for(int i = 0; i < 1000000; ++i) {
      tt += i;
    }
    int r3;
    //std::cout << "remove this " << kvc.pull<int>(key) << std::endl;
    //std::cout << "remove this " << kvc.pull<int>(key) << std::endl;
    std::cout << "not remove this " << kvc.pull<int>(key2) << std::endl;
    std::cout << "not contain " << kvc.contains(key) << std::endl;
    std::cout << "contain " << kvc.contains(key2) << std::endl;
    kvc.clear();
    for(int i = 0; i < 1000000; ++i) {
      tt += i;
    }
    //std::cout << "after clear " << kvc.pull<int>(key2) << std::endl;
    //std::cout << "after clear " << kvc.pull<int>(key2) << std::endl;
    //std::cout << "after clear " << kvc.pull<int>(key2) << std::endl;
  }
  {
    paracel::str_type key7 = "key";
    paracel::str_type val7 = "abcsadsABCDrv";
    //paracel::str_type val7 = "PARACELparasolPARASOL";
    auto r = kvc.push(key7, val7);
    std::cout << r << std::endl;
    auto r2 = kvc.pull<paracel::str_type>(key7);
    std::cout << "*" << r2 << "*"<< std::endl;
  }
  {
    paracel::str_type key8 = "key8";
    std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> target1 = {1.11, 2.22, 3.33};
    paracel::list_type<double> target2 = {3.33, 2.22, 1.11};
    d["key_0"] = target1;
    d["key_1"] = target2;
    kvc.push(key8, d);
    auto r = kvc.pull<std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > >(key8);
    for(auto & v : r) {
      std::cout << v.first << ":";
      for(auto & val : v.second) {
        std::cout << val << "|";
      }
      std::cout << std::endl;
    }
  }
  {
  }
  return 0;
}
