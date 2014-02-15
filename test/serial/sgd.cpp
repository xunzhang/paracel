#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

#include "ps.hpp"
#include "utils.hpp"

using std::string;
using std::vecotr;
using std::random_shuffle;
using paracel::paralg;
using paracel::Comm;
using paracel::str_split;
using paracel::random_double_list;
using paracel::dot_product;

namespace paracel {

class sgd {

public:  
  sgd(Comm comm,
  	std::string _input, 
  	std::string _output, 
	size_t _rounds = 1, 
	double _alpha = 0.002, 
	double _beta = 0.1,
	bool _debug = false) : 
		input(_input), 
		output(_output), 
		rounds(_rounds), 
		alpha(_alpha), 
		beta(_beta),
		debug(_debug) {
    pt = new paralg(comm, output, rounds);
  }
  
  ~sgd() {
    delete pt;
  }
 
  // logistic regression hypothesis function
  // e ** (v .dot theta) / (1 + e ** (v. dot theta))
  double lg_hypothesis(const vector<double> & v) {
    double dp = dot_product(v, theta);
    double temp = 1. / (1. + exp(dp));
    return exp(dp) * temp;
  }

  // set samples and labels member
  void local_parser(const vector<string> & linelst, const char sep = ',') {
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
    int data_sz = samples.size(); data_dim = samples[0].size();
    theta = random_double_list(data_dim);
    vector<int> idx;
    for(int i = 0; i < data_sz; ++i) {
      idx.push_back(i);
    }
    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      random_shuffle(inx.begin(), idx.end());
      for(auto id : idx) {
        double grad = labels[id] - lg_hypothesis(samples[id]);
	double opt1 = alpha * grad;
	double opt2 = 2. * beta * alpha;
        for(int i = 0; i < data_dim; ++i) {
          theta[i] += opt1 * samples[id][i] - opt2 * theta[i];
	  //thera[i] += alpha * grad * samples[id][i] - 2. * beta * alpha * theta[i];
        }
	if(debug) {
	  loss_error.push_back(calc_loss());
	}
      } // end traverse
    } // end rounds
  }

  void solve() {
    auto lines = pt->paracel_load(intput);
    local_parser(lines);
    learning();
    dump_result();
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
    pt->dump_vector(theta, "lg_theta_" + '|');
    pt->dump_vector(error, "lg_loss_error_", ',');
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

int main(int argc, char *argv[])
{
  paracel::sgd sgd_solver;
  sgd_solver.learning();
  return 0;
}
