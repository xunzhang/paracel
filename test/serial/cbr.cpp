#include <vector>
#include <string>
#include <unordered_map>
#include <google/gflags.h>
#include "ps.hpp"
#include "graph.hpp"
#include "utils.hpp"

using namespace std;
using namespace paracel;

namespace paracel {

class cbr {
 public:
  cbr(Comm comm,
      string _input_rating,
      string _input_ibias,
      string _input_ifac,
      string _output,
      int _rounds = 1, 
      double _alpha = 0.005,
      double _beta = 0.01) : rating_input(_input_rating),
                        ibias_input(_input_ibias),
                        ifac_input(_input_ifac),
                        output(_output),
                        rounds(_rounds),
                        alpha(_alpha),
                        beta(_beta) {
    pt = new paralg(comm, output, rounds);
  }
  
  ~cbr() {
    delete pt;
  }

  void init() {
    
    miu = 3.6033;
    
    // load ibias
    auto lines = pt->paracel_load(ibias_input);
    auto local_ibias_parser = [&] (const vector<string> & linelst) {
      for(auto & line : linelst) {
        auto v = str_split(line, '\t');
        ibias[v[0]] = std::stod(v[1]);
      }
    };
    local_ibias_parser(lines);

    // load ifactor
    lines = pt->paracel_load(ifac_input);
    auto local_ifac_parser = [&] (const vector<string> & linelst) {
      auto tmp1 = paracel::str_split(linelst[0], '\t');
      auto tmp2 = paracel::str_split(tmp1[1], '|');
      fac_dim = tmp2.size() + 1;
      for(auto & line : linelst) {
        vector<double> tmp;
        tmp.push_back(1.);
        auto v = paracel::str_split(line, '\t');
        auto vv = paracel::str_split(v[1], '|');
        for(size_t i = 0; i < vv.size(); ++i) {
          tmp.push_back(std::stod(vv[i]));
        }
        ifactor[v[0]] = tmp;
      }
    };
    local_ifac_parser(lines);
    lines.resize(0);
    
    // load user rating list
    auto local_rating_parser = [] (const string & line) {
      return str_split(line, ',');
    };
    auto rating_parser = gen_parser(local_rating_parser);
    rating_graph = new bigraph<string>;
    pt->paracel_load_as_graph(*rating_graph, 
                              rating_input, 
                              rating_parser);
    auto split_lambda = [&] (const string & a,
                             const string & b,
                             double c) {
      usr_rating_lst[a].push_back(
          std::make_pair(b, c)
          );
    };
    rating_graph->traverse(split_lambda);
    delete rating_graph;

    // init ubias, ufactor
    for(auto & kv : usr_rating_lst) {
      ufactor[kv.first] = random_double_list(fac_dim, 0.0001);
      ubias[kv.first] = 0.;//0.01 * random_double();
    }
  }

  void learning() {
    init();
    pt->sync();
    // learning
    for(int rd = 0; rd < rounds; ++rd) {
      for(auto & meta : usr_rating_lst) {
        auto uid = meta.first;
        for(auto & kv : meta.second) {
          auto iid = kv.first;
          auto wgt = kv.second;
          if(ifactor.count(iid) == 0) {
            std::cout << "bug" << std::endl;
          }
          double e = wgt - miu - ibias[iid] - ubias[uid] - dot_product(ufactor[uid], ifactor[iid]);
          ufactor[uid][0] += alpha * (e * ifactor[iid][0]);
          for(int k = 1; k < fac_dim; ++k) {
            double reg_delta = beta * ufactor[uid][k];
            ufactor[uid][k] += alpha * (e * ifactor[iid][k] - reg_delta);
          }
          ubias[uid] += alpha * (e - beta * ubias[uid]);
        }
      }
    }
    usr_rating_lst.clear();
    ifactor.clear();
    ibias.clear();
  }

  void solve() {
    learning();
  }

  void dump_result() {
    pt->paracel_dump_dict(ufactor, "W_");
    pt->paracel_dump_dict(ubias, "ubias_");
  }

 private:
  string rating_input, ibias_input, ifac_input;
  string output;
  paralg *pt;
  int fac_dim, rounds;
  double alpha, beta;

  double miu;
  bigraph<string> *rating_graph;
  unordered_map<string, vector<std::pair<string, double> > > usr_rating_lst;
  unordered_map<string, vector<double> > ifactor, ufactor;
  unordered_map<string, double> ibias, ubias;
};

} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  
  google::SetUsageMessage("[options]\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser jp(FLAGS_cfg_file);
  string input_rating = jp.parse<string>("input_rating");
  string input_ibias = jp.parse<string>("input_ibias");
  string input_ifac = jp.parse<string>("input_ifac");
  string output = jp.parse<string>("output");
  double alpha = jp.parse<double>("alpha");
  double beta = jp.parse<double>("beta");
  int rounds = jp.parse<int>("rounds");

  paracel::cbr solver(comm, 
                      input_rating, 
                      input_ibias, 
                      input_ifac,
                      output, 
                      rounds, 
                      alpha, 
                      beta);
  solver.solve();
  solver.dump_result();
  return 0;
}
