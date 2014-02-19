#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

#include <google/gflags.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "ps.hpp"
#include "utils.hpp"

using std::string;
using std::vector;
using std::random_shuffle;
using paracel::paralg;
using paracel::Comm;
using paracel::str_split;
using paracel::random_double_list;
using paracel::dot_product;
using namespace boost::property_tree;

namespace paracel {

class logistic_regression {

public:  
  logistic_regression(Comm comm,
  	std::string _input, 
  	std::string _output, 
	size_t _rounds = 1, 
	double _alpha = 0.00005, 
	double _beta = 0.01,
	bool _debug = false) : 
		input(_input), 
		output(_output), 
		rounds(_rounds), 
		alpha(_alpha), 
		beta(_beta),
		debug(_debug) {
    pt = new paralg(comm, output, rounds);
  }
  
  ~logistic_regression() {
    delete pt;
  }
 
  // logistic regression hypothesis function
  // e ** (v .dot theta) / (1 + e ** (v. dot theta))
  double lg_hypothesis(const vector<double> & v) {
    double dp = dot_product(v, theta);
    return 1. / (1. + exp(-dp));
  }

  // set samples and labels member
  void local_parser(const vector<string> & linelst, const char sep = ',') {
    samples.resize(0);
    labels.resize(0);
    for(auto & line : linelst) {
      vector<double> tmp;
      double label;
      auto linev = str_split(line, sep);
      tmp.push_back(1.);
      for(int i = 0; i < linev.size() - 1; ++i) {
        tmp.push_back(std::stod(linev[i]));
      }
      samples.push_back(tmp);
      labels.push_back(std::stod(linev[linev.size() - 1]));
    }
  }

  void learning() {
    int data_sz = samples.size(), data_dim = samples[0].size();
    std::cout << data_sz << std::endl;
    std::cout << data_dim << std::endl;
    theta = random_double_list(data_dim);
    vector<int> idx;
    for(int i = 0; i < data_sz; ++i) {
      idx.push_back(i);
    }
    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      random_shuffle(idx.begin(), idx.end());
      std::cout << "round:" << rd << std::endl;
      double opt2 = 2. * beta * alpha;
      vector<double> delta(data_dim);
      for(int i = 0; i < data_dim; ++i) {
        delta[i] = 0.;
      }
      for(auto id : idx) {
        for(int i = 0; i < data_dim; ++i) {
          double opt1 = alpha * (labels[id] - lg_hypothesis(samples[id]));
	  double t = opt1 * samples[id][i] - opt2 * theta[i];
	  delta[i] += t;
	  //theta[i] += t;
        }
	if(debug) {
	  loss_error.push_back(calc_loss());
	}
      } // end traverse
      for(int i = 0; i < data_dim; ++i) {
        theta[i] += delta[i];
      }
      std::cout << calc_loss() / data_sz << std::endl;
    } // end rounds
  }

  void solve() {
    auto lines = pt->paracel_load(input);
    local_parser(lines);
    learning();
    //dump_result();
  }
  
  void print(const vector<double> & vl) {
    for(auto & v : vl) {
      std::cout << v << "|";
    }
    std::cout << std::endl;
  }

  double calc_loss() {
    double loss = 0.;
    for(int i = 0; i < samples.size(); ++i) {
      double j = lg_hypothesis(samples[i]);
      loss += j * j;
    }
    return loss;  
  }

  void dump_result() {
    pt->dump_vector(theta, "lg_theta_", "|");
    pt->dump_vector(loss_error, "lg_loss_error_", "\n");
  }

  void predict() {
    std::string p_fn = "/mfs/user/wuhong/paracel/data/classification/test_000.csv";
    //auto lines = pt->paracel_load("/mfs/user/wuhong/paracel/data/classification/test_000.csv"); 
    auto lines = pt->paracel_load(p_fn);
    local_parser(lines);
    std::cout << "mean loss" << calc_loss() / samples.size() << std::endl;
  }
   
private:
  size_t rounds;
  double alpha, beta;
  string input, output;
  vector<vector<double> > samples;
  vector<double> labels, theta;
  paralg *pt;
  bool debug = false;
  vector<double> loss_error;
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
  double alpha = pt.get<double>("alpha");
  double beta = pt.get<double>("beta");
  int rounds = pt.get<int>("rounds");
  paracel::logistic_regression lg_solver(comm, input, output, rounds, alpha, beta, false);
  lg_solver.solve();
  //lg_solver.dump_result();
  lg_solver.predict();
  return 0;
}
