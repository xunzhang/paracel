#include <string>
#include <vector>
#include <iostream>

#include <google/gflags.h>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

using std::string;
using std::vector;
using paracel::paralg;
using paracel::Comm;
using paracel::gen_parser;

namespace paracel {

auto local_parser = [] (const std::string & line) {
  return paracel::str_split(line, ',');
};

class similarity {

public:
  similarity(Comm comm,
  string _input,
  string _output,
  int _k = 100) :
  	input(_input),
	output(_output),
	ktop(_k) {
    pt = new paralg(comm, output, 1);
  }
  
  ~similarity() {
    delete pt;
  }
  
  void init_paras() {
    auto f_parser = gen_parser(local_parser);
	pt->paracel_load_as_graph(input, f_parser);
  }
  
  void learning() {}
  
  void solve() {
    init_paras();
	learning();
  }

  void dump_result() {}

private:
  string input, output;
  int ktop;
  vector<vector<double> > vects;
  vector<double> sim_wgts;
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
  std::string input = jp.parse<std::string>("input");
  std::string output = jp.parse<std::string>("output");
  int ktop = jp.parse<int>("topk");

  //paracel::similarity sim_solver();
  //sim_solver.solve();
  //sim_solver.dump_result();
  return 0;
}
