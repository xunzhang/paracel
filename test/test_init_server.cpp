#include <iostream>

#include "server.hpp"
#include "paracel_types.hpp"
#include "utils.hpp"

int main(int argc, char *argv[])
{
  paracel::str_type s = "beater7";
  paracel::init_thrds(s, "7773");
  return 0;
}
