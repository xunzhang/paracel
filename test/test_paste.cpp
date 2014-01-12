#include <iostream>
#include <string>
#include <vector>
#include "paracel_types.hpp"
#include "packer.hpp"

// terminate function for recursive variadic template
template<class T>
paracel::str_type paste(const T & arg) {
  paracel::packer<T> pk(arg);
  paracel::str_type scrip;
  pk.pack(scrip);
  return scrip;
}

// T must be paracel::str_type
// use template T to do recursive variadic
template<class T, class ...Args>
T paste(const T & op_str, const Args & ...args) { 
  paracel::packer<T> pk(op_str);
  T scrip;
  pk.pack(scrip); // pack to scrip
  return scrip + paracel::seperator + paste(args...); 
}


int main(int argc, char *argv[])
{
  std::string key = "p[i,:]";
  std::string op_str = "pull";
  std::cout << paste(op_str, key) << std::endl;
  std::cout << paste(std::string("pull"), key) << std::endl;
  std::string v = "val";
  std::cout << paste(std::string("push"), key, v) << std::endl;
  std::vector<double> v2 = {1., 2., 3., 4., 5.};
  std::cout << paste(std::string("push"), key, v2) << std::endl;
  return 0;
}
