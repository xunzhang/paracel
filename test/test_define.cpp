#include <iostream>
#include "paracel_types.hpp"

#define BLK_SZ 8

namespace test {

void foo() {
  std::cout << BLK_SZ << std::endl;
}

}

int main(int argc, char *argv[]) {
  test::foo();
  return 0;
}
