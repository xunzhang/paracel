#include <string>
#include <iostream>
#include <functional>
#include "utils.hpp"
#include "paracel_types.hpp"

int main(int argc, char *argv[])
{
  paracel::hash_type<int> hfunc;
  std::cout << hfunc(0) << std::endl;
  std::cout << hfunc(1) << std::endl;
  std::cout << hfunc(2) << std::endl;
  std::cout << hfunc(3) << std::endl;
  {
    paracel::hash_type<std::string> hfunc;
    std::cout << hfunc("0") << std::endl;
    std::cout << hfunc("1") << std::endl;
    std::cout << hfunc("2") << std::endl;
    std::cout << hfunc("3") << std::endl;
    std::cout << hfunc("4") << std::endl;
    std::cout << hfunc("5") << std::endl;
    std::cout << hfunc("6") << std::endl;

    std::cout << "---" << std::endl;
  
    std::hash<std::string> hfunc2;
    std::cout << hfunc2("0") << std::endl;
    std::cout << hfunc2("1") << std::endl;
    std::cout << hfunc2("2") << std::endl;
    std::cout << hfunc2("3") << std::endl;
  }
  {
    std::cout << "mod 3" << std::endl;
    paracel::hash_type<std::string> hfunc;
    std::cout << hfunc("0") % 3 << std::endl;
    std::cout << hfunc("1") % 3 << std::endl;
    std::cout << hfunc("2") % 3 << std::endl;
    std::cout << hfunc("3") % 3 << std::endl;
    std::cout << hfunc("4") % 3 << std::endl;
    std::cout << hfunc("5") % 3 << std::endl;
    std::cout << hfunc("6") % 3 << std::endl;

    std::cout << "---" << std::endl;
  
    std::hash<std::string> hfunc2;
    std::cout << hfunc2("0") % 3 << std::endl;
    std::cout << hfunc2("1") % 3 << std::endl;
    std::cout << hfunc2("2") % 3 << std::endl;
    std::cout << hfunc2("3") % 3 << std::endl;
  }
  {
    std::cout << "mod 4" << std::endl;
    paracel::hash_type<std::string> hfunc;
    std::cout << hfunc("0") % 4 << std::endl;
    std::cout << hfunc("1") % 4 << std::endl;
    std::cout << hfunc("2") % 4 << std::endl;
    std::cout << hfunc("3") % 4 << std::endl;
    std::cout << hfunc("4") % 4 << std::endl;
    std::cout << hfunc("5") % 4 << std::endl;
    std::cout << hfunc("6") % 4 << std::endl;

    std::cout << "---" << std::endl;
  
    std::hash<std::string> hfunc2;
    std::cout << hfunc2("0") % 4 << std::endl;
    std::cout << hfunc2("1") % 4 << std::endl;
    std::cout << hfunc2("2") % 4 << std::endl;
    std::cout << hfunc2("3") % 4 << std::endl;
  }
  return 0;
}
