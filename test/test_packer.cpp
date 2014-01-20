#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <tr1/unordered_map>
#include "paracel_types.hpp"
#include "packer.hpp"
//#include "msgpack/type/tr1/unordered_map.hpp"

int main(int argc, char *argv[])
{
  {
    paracel::packer<bool> obj(true);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
  }
  {
    paracel::packer<bool> obj(false);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
  }
  {
    paracel::list_type<paracel::str_type> target = {"hello", "world"};
    paracel::packer<paracel::list_type<paracel::str_type> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      std::cout << v << std::endl;
    }
  }
  {
    paracel::str_type target = "PARACEL";
    paracel::packer<paracel::str_type> obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    std::cout << obj.unpack(s) << std::endl;
  }
  {
    paracel::list_type<paracel::str_type> target = {"hello", "world"};
    paracel::packer<paracel::list_type<paracel::str_type> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      std::cout << v << std::endl;
    }
  }
  {
    paracel::str_type target("test,lol");
    paracel::packer<> obj(target);
    //paracel::packer<paracel::str_type> obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
  }
  {
    double target = 3.14;
    paracel::packer<double> obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
  }
  {
    paracel::list_type<int> target = {77, 88};
    paracel::packer<paracel::list_type<int> > obj(target);
    std::string s;
    obj.pack(s);
    std::cout << s << std::endl;
    auto r = obj.unpack(s);
    for(auto & v : r) {
      std::cout << "debug" << v << std::endl;
    }
  }
  {
    paracel::list_type<double> target = {1., 2., 3.};
    paracel::packer<paracel::list_type<double> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      std::cout << v << std::endl;
    }
  }
  {
    paracel::list_type<double> target = {1., 2., 3.};
    paracel::packer<paracel::list_type<double> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r)
      std::cout << v << std::endl;
  }
  {
    paracel::list_type<float> target = {1.1, 2.2, 3.3};
    paracel::packer<paracel::list_type<float> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      std::cout << v << std::endl;
    }
  }
  {
    paracel::list_type<float> target = {1.1, 2.2, 3.3};
    paracel::packer<paracel::list_type<float> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r)
      std::cout << v << std::endl;
  }
  {
    std::map<paracel::str_type, paracel::list_type<float> > d;
    paracel::list_type<float> target1 = {1.1, 2.2, 3.3};
    paracel::list_type<float> target2 = {3.3, 2.2, 1.1};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::map<paracel::str_type, paracel::list_type<float> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::tr1::unordered_map<int, int> d;
    d[1] = 1;
    d[2] = 2;
    paracel::packer<std::tr1::unordered_map<int, int> > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
        std::cout << "#" << v.first << ":" << v.second << "#" << std::endl;
    }
  }
  {
    std::tr1::unordered_map<paracel::str_type, paracel::list_type<float> > d;
    paracel::list_type<float> target1 = {1.1, 2.2, 3.3};
    paracel::list_type<float> target2 = {3.3, 2.2, 1.1};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::tr1::unordered_map<paracel::str_type, paracel::list_type<float> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> target1 = {1.11, 2.22, 3.33};
    paracel::list_type<double> target2 = {3.33, 2.22, 1.11};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> target1 = {7.77, 8.88, 9.99};
    paracel::list_type<double> target2 = {9.99, 8.88, 7.77};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::unordered_map<paracel::str_type, paracel::list_type<double> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  return 0;
}
