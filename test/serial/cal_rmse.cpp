#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "utils.hpp"

using namespace std;

void load_ifactor(unordered_map<
            string, vector<double> 
            > & var, 
            string & fn) {
  ifstream f(fn);
  string line_buf;
  while(getline(f, line_buf)) {
    auto tmp = paracel::str_split(line_buf, "\t");
    auto tmp2 = paracel::str_split(tmp[1], "|");
    vector<double> temp;
    temp.push_back(1.);
    for(auto & fac : tmp2) {
      temp.push_back(std::stod(fac));
    }
    var[tmp[0]] = temp;
  }
  f.close();
}

void load_ufactor(unordered_map<
            string, vector<double> 
            > & var, 
            string & fn) {
  ifstream f(fn);
  string line_buf;
  while(getline(f, line_buf)) {
    auto tmp = paracel::str_split(line_buf, "\t");
    auto tmp2 = paracel::str_split(tmp[1], "|");
    vector<double> temp;
    for(auto & fac : tmp2) {
      temp.push_back(stod(fac));
    }
    var[tmp[0]] = temp;
  }
  f.close();
}

void load_bias(unordered_map<
               string, double
               > & var,
               string & fn) {
  ifstream f(fn);
  string line_buf;
  while(getline(f, line_buf)) {
    auto tmp = paracel::str_split(line_buf, "\t");
    var[tmp[0]] = stod(tmp[1]);
  }
  f.close();
}

void load_miu(double & miu, 
              string & fn) {
  ifstream f(fn);
  string line_buf;

  getline(f, line_buf);
  getline(f, line_buf);
  auto tmp = paracel::str_split(line_buf, "\t");
  miu = stod(tmp[1]);
  f.close();
}

double cal_rmse(string & in_miu,
               string & in_ibias,
               string & in_ifac,
               string & in_ubias,
               string & in_ufac,
               string & in_test) {
  double miu;
  unordered_map<string, double> ibias, ubias;
  unordered_map<string, vector<double> > ifac, ufac;
  load_miu(miu, in_miu);
  load_bias(ibias, in_ibias);
  load_bias(ubias, in_ubias);
  load_ifactor(ifac, in_ifac);
  load_ufactor(ufac, in_ufac);
  
  auto estimate = [&] (const string & uid, 
                       const string & iid) {
    return miu + 
        ubias[uid] + ibias[iid] + 
        paracel::dot_product(ufac[uid], ifac[iid]);
  };

  ifstream f_test(in_test);
  string line_buf;
  double rmse = 0.;
  size_t rating_sz = 0;
  while(getline(f_test, line_buf)) {
    auto tmp = paracel::str_split(line_buf, " ");
    string uid = tmp[0];
    string iid = tmp[1];
    double rating = stod(tmp[3]);
    double err = rating - estimate(uid, iid);
    rmse += err * err;
    rating_sz += 1;
  }
  f_test.close();
  return sqrt(rmse / rating_sz);
}

double cal_rmse_mix(string & in_miu,
                    string & in_ibias,
                    string & in_ifac,
                    string & in_ubias1,
                    string & in_ufac1,
                    string & in_ubias2,
                    string & in_ufac2,
                    string & in_test) {
  double miu;
  unordered_map<string, double> ibias, ubias1, ubias2;
  unordered_map<string, vector<double> > ifac, ufac1, ufac2;
  load_miu(miu, in_miu);
  load_bias(ibias, in_ibias);
  load_bias(ubias1, in_ubias1);
  load_bias(ubias2, in_ubias2);
  load_ifactor(ifac, in_ifac);
  load_ufactor(ufac1, in_ufac1);
  load_ufactor(ufac2, in_ufac2);
  
  auto estimate1 = [&] (const string & uid, 
                       const string & iid) {
    return miu + 
        ubias1[uid] + ibias[iid] + 
        paracel::dot_product(ufac1[uid], ifac[iid]);
  };
  
  auto estimate2 = [&] (const string & uid, 
                       const string & iid) {
    return miu + 
        ubias2[uid] + ibias[iid] + 
        paracel::dot_product(ufac2[uid], ifac[iid]);
  };

  ifstream f_test(in_test);
  string line_buf;
  double rmse = 0.;
  size_t rating_sz = 0;
  while(getline(f_test, line_buf)) {
    auto tmp = paracel::str_split(line_buf, " ");
    string uid = tmp[0];
    string iid = tmp[1];
    double rating = stod(tmp[3]);
    double err = rating - (estimate1(uid, iid) * 0.3 + estimate2(uid, iid) * 0.7);
    rmse += err * err;
    rating_sz += 1;
  }
  f_test.close();
  return sqrt(rmse / rating_sz);
}

int main(int argc, char *argv[])
{
  string miu = "/mfs/user/wuhong/paracel/data/netflix_result7/miu_0";
  string ibias = "/mfs/user/wuhong/paracel/data/netflix_result7/ibias_0";
  string ifac = "/mfs/user/wuhong/paracel/data/netflix_result7/H_0";
  string ubias = "/mfs/user/wuhong/paracel/data/netflix_result7/ubias_0";
  string ufac = "/mfs/user/wuhong/paracel/data/netflix_result7/W_0";
  string cbr_ubias = "/mfs/user/wuhong/paracel/data/cbr_result4/ubias_0";
  string cbr_ufac = "/mfs/user/wuhong/paracel/data/cbr_result4/W_0";
  string mix_ubias = "/mfs/user/wuhong/paracel/test/serial/mix_ubias_netflix";
  string mix_ufac = "/mfs/user/wuhong/paracel/test/serial/mix_ufactor_netflix";
  string test = "/mfs/user/wuhong/paracel/data/netflix/test/test";
  //string test = "/mfs/user/wuhong/paracel/data/netflix/train/train";
  //std::cout << cal_rmse(miu, ibias, ifac, ubias, ufac, test) << std::endl;
  std::cout << cal_rmse(miu, ibias, ifac, cbr_ubias, cbr_ufac, test) << std::endl;
  //std::cout << cal_rmse(miu, ibias, ifac, mix_ubias, mix_ufac, test) << std::endl;
  //std::cout << cal_rmse_mix(miu, ibias, ifac, ubias, ufac, cbr_ubias, cbr_ufac, test) << std::endl;
  return 0;
}
