#include <functional>
#include <vector>
#include <string>
#include "paracel_types.hpp"
#include "proxy.hpp"
#include "utils.hpp"

using update_result = std::function<std::string(std::string, std::string)>;
using filter_result = std::function<bool(std::string, std::string)>;

extern "C" {
  extern update_result local_update;
  extern filter_result local_filter;
}

double foo(double a, double b) { 
  return a * b; 
}

bool goo0(std::string k, double v) {
  if(v >= 10.) { return true; }
  return false;
}

bool goo1(std::string k, double v) {
  paracel::str_type s = "3";
  if(paracel::endswith(k, s)) {  return true; }
  return false;
}

update_result local_update = paracel::update_proxy(foo);
filter_result local_filter = paracel::filter_proxy(goo0);
