/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef FILE_9e61b185_5614_5b9c_fb78_2abb59d42119_HPP
#define FILE_9e61b185_5614_5b9c_fb78_2abb59d42119_HPP

#include <vector>
#include <chrono>

#include "ps.hpp"
#include "utils.hpp"

#define N 20000000

namespace paracel {

class perf : public paracel::paralg {

 public:
  perf(paracel::Comm comm, 
       std::string hosts_dct_str, 
       std::string _output) : 
    paracel::paralg(hosts_dct_str, comm, _output) {
      /*
      for(size_t i = 0; i < N; ++i) {
        data.push_back(3.14);
      }
      for(size_t i = 0; i < N; ++i) {
        data2.push_back("勋章的搁浅ffof");
      }
      */
      for(size_t i = 0; i < N; ++i) {
        data3["搁浅的勋章" + std::to_string(i)] = 1;
      }
    }
  
  virtual ~perf() {}

  void solve() {
    auto start = std::chrono::high_resolution_clock::now();
    paracel_write("key4write", data3);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(end - start);
    std::cout << "paracel_write time for 300MB data is: " << elapsed.count() << std::endl;
    auto result = paracel_read<std::unordered_map<std::string, int> >("key4write");
    //std::cout << "data: " << result[0] << "|" << result.size() << std::endl;
    start = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(start - end);
    std::cout << "paracel_read time for 300MB data is: " << elapsed.count() << std::endl;
    paracel_bupdate("key4bupdate", 
                    data3, 
                    "/mfs/user/wuhong/paracel/local/lib/libperf_update.so", 
                    "perf_updater");
    sync();
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(end - start);
    std::cout << "paracel_bupdate time for 300MB data is: " << elapsed.count() << std::endl;
  }
    
 private:
  std::vector<double> data;
  std::vector<std::string> data2;
  std::unordered_map<std::string, int> data3;
}; // class perf

} // namespace paracel

#endif
