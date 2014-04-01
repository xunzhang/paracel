#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>

#include <google/gflags.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "graph.hpp"
#include "ps.hpp"
#include "utils.hpp"

using paracel::paralg;
using paracel::Comm;
using paracel::dot_product;
using paracel::random_double_list;
using paracel::random_double;
using namespace boost::property_tree;
using paracel::gen_parser;

namespace paracel {

auto local_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
};

class matrix_factorization {

 public:
  matrix_factorization (Comm comm, 
                        std::string _input, 
                        std::string _output,
                        int _k = 100,
                        size_t _rounds = 1,
                        double _alpha = 0.005,
                        double _beta = 0.01,
                        bool _debug = false) : 
      input(_input),
      output(_output),
      k(_k),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta),
      debug(_debug) {
        pt = new paralg(comm, output, rounds);
      }

  ~matrix_factorization() {
    delete pt;
  }

  void load() {
    // init data info
    auto f_parser = gen_parser(local_parser);
    pt->paracel_load_as_graph(rating_graph, input, f_parser);
    std::cout << "load done" << std::endl;
    rating_sz = rating_graph.e();
    auto init_lambda = [&] (const std::string & a, 
                            const std::string & b, 
                            double c) { 
      usr_dct[a] = a;
      item_dct[b] = b;
      miu += c;
    };
    rating_graph.traverse(init_lambda);
    miu /= rating_sz;
    std::cout << miu << std::endl;
  }

  inline double estimate(const std::string & uid, const std::string & iid) {
    return miu + usr_bias[uid] + item_bias[iid] + dot_product(p[uid], q[iid]);
  }

  double cal_rmse() {
    rmse = 0.;
    auto rmse_lambda = [&] (const std::string & uid,
                            const std::string & iid,
                            double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    rating_graph.traverse(rmse_lambda);
    return sqrt(rmse / rating_sz);
  }

  void learning() {
    std::vector<double> delta_p(k), delta_q(k);
    for(auto & kv : usr_dct) {
      p[kv.second] = random_double_list(k, 0.1);
      usr_bias[kv.second] = 0.1 * random_double();
    }
    for(auto & kv : item_dct) {
      q[kv.second] = random_double_list(k, 0.1);
      item_bias[kv.second] = 0.1 * random_double();
    }
    std::cout << "init done" << std::endl;

    auto learning_lambda = [&] (const std::string & uid,
                                const std::string & iid,
                                double rating) {
      double e = rating - estimate(uid, iid);
      // compute delta
      for(int i = 0; i < k; ++i) {
        delta_p[i] = alpha * (2 * e * q[iid][i] - beta * p[uid][i]);
        delta_q[i] = alpha * (2 * e * p[uid][i] - beta * q[iid][i]);
      }
      // update with delta
      for(int i = 0; i < k; ++i) {
        p[uid][i] += delta_p[i];
        q[iid][i] += delta_q[i];
      }
      // update bias
      usr_bias[uid] += alpha * (2 * e - beta * usr_bias[uid]);
      item_bias[iid] += alpha * (2 * e - beta * item_bias[iid]);
    };

    // learning
    for(int rd = 0; rd < rounds; ++rd) {
      std::cout << "rd:" << rd << " started" << std::endl;
      rating_graph.traverse(learning_lambda);
    } // end for
  }

  void solve() {
    load();
    learning();
  }

 private:
  paralg *pt;
  size_t rounds;
  bool debug;
  int k;
  double alpha, beta;
  std::string input, output;
  std::vector<double> loss_error;

  int rating_sz = 0;
  double miu = 0., rmse = 0.;
  paracel::bigraph<std::string> rating_graph;

  std::unordered_map<std::string, std::string> usr_dct, item_dct;
  std::unordered_map<std::string, std::vector<double> > p, q;
  std::unordered_map<std::string, double> usr_bias, item_bias;
};

} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  ptree pt;
  json_parser::read_json(FLAGS_cfg_file, pt);
  std::string input = pt.get<std::string>("input");
  std::string output = pt.get<std::string>("output");
  int k = pt.get<int>("k");
  double alpha = pt.get<double>("alpha");
  double beta = pt.get<double>("beta");
  int rounds = pt.get<int>("rounds");
  paracel::matrix_factorization mf_solver(comm, input, output, k, rounds, alpha, beta, false);
  mf_solver.solve();
  std::cout << "solved" << std::endl;
  std::cout << mf_solver.cal_rmse() << std::endl;
  return 0;
}
