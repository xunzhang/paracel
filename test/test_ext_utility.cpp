#include <string>
#include <iostream>
#include <vector>

#include "utils/ext_utility.hpp"
#include "paracel_types.hpp"

int main(int argc, char *argv[])
{
  {
    std::cout << "---------" << std::endl;
    std::string a("ancs,");
    auto res = paracel::str_split(a, "|");
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a|bc|d|");
    auto res = paracel::str_split(a, '|');
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a|bc|d");
    auto res = paracel::str_split(a, '|');
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a|bc||d");
    auto res = paracel::str_split(a, '|');
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a,bc|d|,,,");
    auto res = paracel::str_split(a, "|,");
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a|bc|d");
    auto res = paracel::str_split(a, "|");
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "---------" << std::endl;
    std::string a("a|bc||d");
    auto res = paracel::str_split(a, '|');
    std::cout << res.size() << std::endl;
    for(auto & s : res) std::cout << s << std::endl;
  }
  {
    std::cout << "--------------" << std::endl;
    std::string a("a b:0.1 c:0.2 d:0.4");
    auto res = paracel::str_split(a, ' ');
    paracel::str_type delimiter("[:| ]*");
    auto p = paracel::str_split(res[1], delimiter);
    std::cout << p.size() << std::endl;
    std::cout << p[0] << " " << p[1] << std::endl;
  }
  {
    std::cout << "-----------" << std::endl;
    std::vector<std::string> aa{"c", "a", "b", "a", "a", "b"};
    auto res = paracel::sort_and_cnt(aa);
    for(auto & item : res) {
      std::cout << item << std::endl;
    }
  }
  return 0;
}
