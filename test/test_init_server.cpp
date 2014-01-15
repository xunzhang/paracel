#include <iostream>

#include "server.hpp"
#include "paracel_types.hpp"

int main(int argc, char *argv[])
{
  paracel::str_type s = "beater7";
  paracel::init_thrds(s);
  return 0;
}
