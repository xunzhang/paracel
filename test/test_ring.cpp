#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "ring.hpp"
#include "utils.hpp"
int main(int argc, char *argv[])
{ 
  {
    std::vector<int> server_names{1, 2, 3};
    paracel::ring<int> ring(server_names);
    std::string key("dw");
    std::cout << ring.get_server(key) << std::endl;
    std::string key2("q[,:0]_0");
    std::cout << ring.get_server(key2) << std::endl;
    std::string key3("q[,:1]_0");
    std::cout << ring.get_server(key3) << std::endl;
    std::string key4("q[,:2]_0");
    std::cout << ring.get_server(key4) << std::endl;
    std::string key5("q[,:3]_0");
    std::cout << ring.get_server(key5) << std::endl;
    std::string key6("p[0:,]_2");
    std::cout << ring.get_server(key6) << std::endl;
    std::string key7("p[13:,]_2");
    std::cout << ring.get_server(key7) << std::endl;
    std::string key8("p[42:,]_2");
    std::cout << ring.get_server(key8) << std::endl;
    std::string key9("p[5:,]_2");
    std::cout << ring.get_server(key9) << std::endl;
    std::string key10("p[3:,]_1");
    std::cout << ring.get_server(key10) << std::endl;
    // char* is unhashable
    //std::cout << ring.get_server("world") << std::endl;
  }
  {
    std::vector<std::string> server_names{"balin1", "beater5", "beater7"};
    paracel::ring<std::string> ring(server_names);
    std::string key("dw");
    std::cout << ring.get_server(key) << std::endl;
    std::string key2("q[,:0]_0");
    std::cout << ring.get_server(key2) << std::endl;
    std::string key3("q[,:1]_0");
    std::cout << ring.get_server(key3) << std::endl;
    std::string key4("q[,:2]_0");
    std::cout << ring.get_server(key4) << std::endl;
    std::string key5("q[,:3]_0");
    std::cout << ring.get_server(key5) << std::endl;
    std::string key6("p[0:,]_2");
    std::cout << ring.get_server(key6) << std::endl;
    std::string key7("p[13:,]_2");
    std::cout << ring.get_server(key7) << std::endl;
    std::string key8("p[42:,]_2");
    std::cout << ring.get_server(key8) << std::endl;
    std::string key9("p[5:,]_2");
    std::cout << ring.get_server(key9) << std::endl;
    std::string key10("p[3:,]_1");
    std::cout << ring.get_server(key10) << std::endl;
  }
  {
    std::vector<int> server_names;
    for(size_t i = 0; i < 100; ++i) {
      server_names.push_back(i);
    }
    paracel::ring<int> ring(server_names);
    std::ifstream f("/home2/jasonzhao/tmp/track_factors");
    std::string line_buf;
    std::unordered_set<std::string> keys;
    while(std::getline(f, line_buf)) {
      auto tmp = paracel::str_split(line_buf, '\t');
      keys.insert(tmp[0]);

    }
    f.close();
    std::unordered_map<int, int> debug;
    for(int i = 0; i < 100; ++i) {
      debug[i] = 0;
    }
    for(auto & tid : keys) {
      for(auto & ttid : keys) {
        for(int k = 0; k < 4; ++k) {
          std::string key = tid + "_" + ttid + "_" + std::to_string(k);
          debug[ring.get_server(key)] += 1;
        }
      }
    }
    for(int i = 0; i < 100; ++i) {
      std::cout << "server " << i << " get " << debug[i] << " keys" << std::endl;
    }
  }
  return 0;
}
