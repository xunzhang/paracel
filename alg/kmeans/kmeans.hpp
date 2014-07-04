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

#ifndef FILE_2fae05e7_4f3a_2ac3_dc77_f92f7070193b_HPP
#define FILE_2fae05e7_4f3a_2ac3_dc77_f92f7070193b_HPP

#include <string>
#include <algorithm>
#include <unordered_map>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>

#include "ps.hpp"
#include "load.hpp"

namespace paracel {

class kmeans : public paracel::paralg {

 public:
  kmeans(paracel::Comm comm,
         std::string hosts_dct_str,
         std::string _input,
         std::string _output, 
         std::string type,
         int k, 
         int _rounds = 1,
         int limit_s = 0) : 
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, limit_s, true),
      input(_input),
      rounds(_rounds),
      dtype(type),
      kclusters(k) {}
 
  virtual ~kmeans() {}

 private:
  // dense matrix
  void local_load_fvec() {}

  // sparse matrix
  void local_load_sim() {}

  void init() {
    if(dtype == "fvec") {
      
      // random pick k sample as init kcluster
      if(get_worker_id() == 0) {
        std::vector<std::string> ids;
        auto get_ids = [&] (const std::vector<std::string> & linelst) {
          for(auto & line : lines) {
            auto r = paracel::str_split(line, ' ');
            ids.push_back(r[0]);
          }
        };
        auto lines = paracel_load(input);
        get_ids(lines); // init ids
        std::random_shuffle(ids.begin(), ids.end());
        ids.resize(kclusters);
      }
      std::unordered_map<std::string, char> map;
      for(size_t i = 0; i < ids.size(); ++i) {
        map[ids[i]] = '7';
      }

      // load local dense matrix
      auto local_parser = [] (const std::string & line) {
        auto r = paracel::str_split(line, ' ');
        auto vec = paracel::str_split(r[1], '|');
        std::copy(vec.begin(), vec.end(), r.end());
        return r;
      };
      auto f_parser = paracel::gen_parser(local_parser);
      //paracel_load_as_matrix(blk_smtx, row_map, input, f_parser);
      paracel_load_as_matrix(blk_smtx, row_map, input, f_parser, "fvec", true);
    
      // push init val into ps
      std::vector<size_t> indxs(kclusters);
      for(auto & kv : row_map) {
        if(map.count(kv.second) != 0) {
          indxs.push_back(kv.first); 
        }
      }
      std::vector<std::vector<double> > init_clusters;
      for(auto & indx : indxs) {
        Eigen::VectorXd row = blk_smtx.row(indx);
        init_clusters.push_back(evec2vec(row));
      }
      paracel_write("init_clusters", init_clusters);
    } else if(dtype == "sim") {
    }
  }

  void learning() {}

 public:
  void solve() {}

 private:
  int kclusters;
  std::string dtype;

  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_smtx;
  Eigen::MatrixXd blk_dmtx;
  std::unordered_map<size_t, std::string> row_map; // matrix_indx -> id
}; // class kmeans

} // namespace paracel

#endif
