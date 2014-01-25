#include <string>
#include <functional>

#include "proxy.hpp"
#include "packer.hpp"

double local_incr(double v, double d) { return v + d; }
using update_result = std::function<std::string(std::string, std::string)>;

void update(std::string key, std::string delta, update_result update_func) {
  std::string val;
  double v = 3.21;
  paracel::packer<double> pk(v);
  pk.pack(val);
  std::string new_val = update_func(val, delta);
  paracel::packer<double> pk2;
  std::cout << pk2.unpack(new_val) << std::endl;
}

int main(int argc, char *argv[])
{
  update_result update_func = paracel::update_proxy(local_incr);
  
  std::string key("hello");
  double a = 1.23;
  paracel::packer<double> pk(a);
  std::string delta;
  pk.pack(delta);

  update(key, delta, update_func);
  
  return 0;
}
