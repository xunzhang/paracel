#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <tr1/unordered_map>
#include <msgpack.hpp>
#include "paracel_types.hpp"
#include "packer.hpp"
#include "graph.hpp"
//#include "msgpack/type/tr1/unordered_map.hpp"
    
struct AA {
 public:
  AA() : a(1), b(1.) {}
  AA(int i, double j) : a(i), b(j) {}
  ~AA() {}
  void dump() {
    std::cout << "a: " << a << " b: " << b << std::endl;
  }
  int a;
  double b;
  MSGPACK_DEFINE(a, b);
};

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
    std::cout << "***************************" << std::endl;
    paracel::packer<int> obj(54);
    std::string s;
    obj.pack(s);
    std::cout << s << std::endl;
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
    std::cout << "***************************" << std::endl;
  }
  {
    std::cout << "***************************" << std::endl;
    paracel::packer<int> obj(51);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    std::cout << r << std::endl;
    std::cout << "***************************" << std::endl;
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
  {
    AA d(7, 3.14);
    paracel::packer<AA> obj(d);
    std::string s;
    obj.pack(s);
    AA r = obj.unpack(s);
    r.dump();
  }
  {
    paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
    tpls.emplace_back(std::make_tuple(0, 0, 3.));
    tpls.emplace_back(std::make_tuple(0, 2, 5.));
    tpls.emplace_back(std::make_tuple(1, 0, 4.));
    tpls.emplace_back(std::make_tuple(1, 1, 3.));
    tpls.emplace_back(std::make_tuple(1, 2, 1.));
    tpls.emplace_back(std::make_tuple(2, 0, 2.));
    tpls.emplace_back(std::make_tuple(2, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 1, 3.));
    tpls.emplace_back(std::make_tuple(3, 3, 1.));
    paracel::bigraph<size_t> bgrp(tpls);
    std::cout << bgrp.v() << std::endl;
    std::cout << bgrp.e() << std::endl;
    std::cout << bgrp.avg_degree() << std::endl;
    auto kk = bgrp.adjacent(0);
    std::cout << "cao" << kk.size() << std::endl;

    paracel::packer<paracel::bigraph<size_t> > obj(bgrp);
    std::string s;
    obj.pack(s);
    paracel::bigraph<size_t> r = obj.unpack(s);
    std::cout << r.v() << std::endl;
    std::cout << r.e() << std::endl;
    std::cout << r.avg_degree() << std::endl;
    auto kkk = r.adjacent(0);
    std::cout << "cao" << kkk.size() << std::endl;
  }
  return 0;
}
