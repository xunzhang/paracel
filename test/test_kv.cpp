#include <iostream>

#include "kv.hpp"
#include "paracel_types.hpp" // paracel_dict_type

int main(int argc, char *argv[])
{
  paracel::kvs<char, int> obj;

  obj.set('a', 2);
  obj.set('b', 0);
  paracel::dict_type<char, int> tmp{std::pair<char, int>('x', 100), std::pair<char, int>('y', 200)};
  obj.set_multi(tmp);

  if(auto v = obj.get('a')) {
    std::cout << *v << std::endl;
  }
  if(auto v = obj.get('b')) {
    std::cout << *v << std::endl;
  }

  int tval;
  obj.get('a', tval);
  std::cout << "a is " << tval << std::endl;
  if(auto v = obj.get('c')) {
    std::cout << *v << std::endl;
  }

  auto dct = obj.getall();
  for(auto & kv : dct) {
    std::cout << kv.first << " " << kv.second << std::endl;
  }
  
  bool r;
  if(auto vv = obj.gets('a')) {
    r = obj.cas('a', 300, 30);//(*vv).second);
    //r = obj.cas('a', 300, (*vv).second);
  }
  if(auto v = obj.get('a'))
    std::cout << "after cas" << *v << std::endl;
  // writing at same time
  while(!r) {
    if(auto vv = obj.gets('a')) {
      r = obj.cas('a', 900, (*vv).second);
    }
  }
  if(auto v = obj.get('a'))
    std::cout << "after cas" << *v << std::endl;

  paracel::kvs< char, std::vector<int> > obj2;
  std::vector<int> ttmp1{1,2,3};
  std::vector<int> ttmp2{1,1,1};
  obj2.set('a', ttmp1);
  obj2.incr('a', ttmp2);
  if(auto v = obj2.get('a'))
    for(auto & item : *v)
      std::cout << "list obj test" << item << std::endl;

  obj.incr('a', 30);
  if(auto v = obj.get('a'))
    std::cout << "new a is " << *v << std::endl;
  
  obj.del('b');
  obj.del('c');
  if(auto v = obj.get('b')) {
    std::cout << *v << std::endl;
  }

  obj.clean();
  if(auto v = obj.get('a')) {
    std::cout << *v << std::endl;
  }

  return 0;
}
