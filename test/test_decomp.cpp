#include <iostream>
#include "utils/decomp.hpp"
using namespace paracel;
int main(int argc, char *argv[]) {
  int x, y;
  npfact2d(10, x, y);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(10, x, y, false);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(7, x, y);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(7, x, y, false);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(2, x, y);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(6, x, y);
  std::cout << "x " << x << " y " << y << std::endl;
  npfact2d(8, x, y);
  std::cout << "x " << x << " y " << y << std::endl;
  return 0;
}
