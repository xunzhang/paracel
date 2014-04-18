#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include "utils.hpp"

using namespace std;

template <class T>
unordered_map<string, vector<double> > 
factor_mix(const T & fn1, 
           const T & fn2,
           double frac = 0.7) {
  
  unordered_map<string, vector<double> > uf1, uf2, uf3;
  
  auto handler_lambda = [] (unordered_map<string, vector<double> > & dct,
                           const string & line) {
    auto tmp = paracel::str_split(line, "\t");
    auto tmp2 = paracel::str_split(tmp[1], "|");
    vector<double> temp;
    for(auto & fac : tmp2) {
      temp.push_back(stod(fac));
    }
    dct[tmp[0]] = temp;
  };

  auto fn_lst1 = paracel::expand(fn1);
  auto fn_lst2 = paracel::expand(fn2);
  string line_buf;
  
  for(auto & ufn1 : fn_lst1) {
    ifstream f1(ufn1);
    while(getline(f1, line_buf)) {
      handler_lambda(uf1, line_buf);
    }
    f1.close();
  }
  for(auto & ufn2 : fn_lst2) {
    ifstream f2(ufn2);
    while(getline(f2, line_buf)) {
      handler_lambda(uf2, line_buf);
    }
    f2.close();
  }
  std::cout << "init done" << std::endl;

  // mix
  assert(uf1.size() == uf2.size());
  for(auto & kv : uf1) {
    vector <double> temp;
    for(size_t k = 0; k < kv.second.size(); ++k) {
      temp.push_back(frac * kv.second[k] + (1 - frac) * uf2[kv.first][k]);
    }
    uf3[kv.first] = temp;
  }
  std::cout << "done" << std::endl;

  return uf3;
}

void dump(const unordered_map<
          string, vector<double> 
          > & ufac,
          const string & output) {
  // dump
  ofstream fout;
  //fout.open(output, ofstream::app);
  fout.open(output);
  std::cout << "dump begin" << std::endl;
  for(auto & kv : ufac) {
    fout << kv.first << '\t';
    for(size_t k = 0; k < kv.second.size() - 1; ++k) {
      fout << kv.second[k] << "|";
    }
    fout << kv.second[kv.second.size() - 1] << '\n';
  }
  std::cout << "dump done" << std::endl;
  fout.close();
}

int main(int argc, char *argv[])
{
  string ufac1 = "/mfs/user/wuhong/paracel/data/netflix_result8/W_*";
  string ufac2 = "/mfs/user/wuhong/paracel/data/cbr_result_parallel/W_*";
  string output = "/mfs/user/wuhong/paracel/test/serial/mix_ufactor_netflix";
  auto mix_ufac = factor_mix(ufac1, ufac2);
  dump(mix_ufac, output);
  return 0;
}
