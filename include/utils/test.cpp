#include <iostream>
#include <string>
#include <vector>
#include <cstring>

bool startswith(const std::string & str, const std::string & key) {
  return str.find(key) == 0;
}

bool endswith(const std::string & str, const std::string & key) {
  return str.rfind(key) == (str.length() - key.length());
}

std::vector<std::string> str_split(const std::string & str, const std::string & seps) {
  std::vector<std::string> result;
  auto last = str.begin();
  auto i = str.begin();
  for(; i != str.end(); ++i) {
    if(std::string(i, i + seps.size()) == seps) {
      result.push_back(std::string(last, i));
      last = i + seps.size();
    }
  }
  if(last != i) {
    result.push_back(std::string(last, i));
  }
  return result;
}


int main(int argc, char *argv[])
{
  std::string s = "a123";
  std::cout << startswith(s, "a") << std::endl;
  auto r = str_split(s, "a")[0];
  std::cout << r << std::endl; 
  
  std::string a = "_3";
  std::string b = "kwy_3";
  std::cout << "debug" << endswith(b, a) << std::endl;

  std::string s2 = "pushPARACELkeyPARACELabcsadsABCDrv";
  auto v = str_split(s2, "PARACEL");
  for(auto & val : v) std::cout << val << std::endl;
  return 0;
}
