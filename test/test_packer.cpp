#include <sstream>
#include <iostream>
#include <string>
#include "paracel_types.hpp"
#include "packer.hpp"

int main(int argc, char *argv[])
{
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
  return 0;
}
