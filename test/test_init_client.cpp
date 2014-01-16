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
  return 0;
}
