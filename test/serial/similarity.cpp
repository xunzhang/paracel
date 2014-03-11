#include <cmath> // std::sqrt std::pow
#include <assert.h>

#include <string>
#include <vector>
#include <iostream>
#include <utility> // std::piar
#include <algorithm> // std::sort

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

  void local_parser(const vector<string> & linelst, 
  				const char sep = ',') {
    for(auto & line : linelst) {
      vector<double> tmp;
	  auto v = paracel::str_split(line, sep);
	  for(size_t i = 1; i < v.size(); ++i) {
	    tmp.push_back(std::stod(v[i]));
	  }
	  item_vects[v[0]] = tmp;
    }
  }
  
  void init_paras() {
	auto lines = pt->paracel_load(input); 
	local_parser(lines);
  }

  void normalize() {
    for(auto & kv : item_vects) {
	  double square_sum = 0.;
	  // calc square sum
	  for(size_t i = 0; i < kv.second.size(); ++i) {
	    square_sum += std::pow(kv.second[i], 2);
	  }
	  // divide
	  for(auto it = kv.second.begin(); it != kv.second.end(); ++it) {
	    *it = *it / std::sqrt(square_sum);
	  }
	}
  }
  
  void learning() {
	// for every item vector - iv
    for(auto & iv : item_vects) {
	  // calc every other item vectos - jv
	  for(auto & jv : item_vects) {
	    if(iv.first != jv.first) {
		  auto key = iv.first + "_" + jv.first;
		  auto rkey = jv.first + "_" + iv.first;
		  bool flag1 = record_map.find(key) != record_map.end();
		  bool flag2 = record_map.find(rkey) != record_map.end();
		  if(flag1 || flag2) {
		    if(flag1) {
		      item_sim_lst[iv.first].push_back(
			  	std::make_pair(jv.first, 
							record_map[key])
				);
			} else {
			  item_sim_lst[iv.first].push_back(
			  	std::make_pair(jv.first,
							record_map[rkey])
				);
			}
		  } else {
		    double sim = paracel::dot_product(iv.second, 
											jv.second);
			item_sim_lst[iv.first].push_back(
			  std::make_pair(jv.first, sim)
			);
			record_map[iv.first + "_" + jv.first] = sim;
		  }
		} 
	  } // for jv
	} // for iv
    
    // get ktop
	for(auto & v : item_sim_lst) {
	  auto item_id = v.first;
	  auto sim_lst = v.second;
	  std::sort(sim_lst.begin(), sim_lst.end(), 
	  [] (const std::pair<string, double> & a,
	      const std::pair<string, double> & b) -> int {
	    return a.second > b.second;
	  });
	  assert(ktop >= (int)sim_lst.size()); // i am not sure = can be deleted here
	  sim_lst.resize(ktop);
	  item_sim_lst[item_id] = sim_lst;
	} // ktop
  }
  
  void dump_result() {
    pt->paracel_dump_dict(item_sim_lst);
  }

  void solve() {
    init_paras();
	normalize(); // normalize here to reduce calculation
	learning();
  }

private:
  string input, output;
  int ktop;
  paracel::dict_type<string, vector<double> > item_vects;
  paracel::dict_type<string, vector<std::pair<string, double> > > item_sim_lst;
  paracel::dict_type<string, double> record_map;
  paralg *pt;
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
  string input = jp.parse<string>("input");
  string output = jp.parse<string>("output");
  int ktop = jp.parse<int>("topk");

  paracel::similarity sim_solver(comm, input, output, ktop);
  sim_solver.solve();
  sim_solver.dump_result();
  return 0;
}
