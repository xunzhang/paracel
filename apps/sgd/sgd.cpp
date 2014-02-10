/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#include <string>
#include "sgd.hpp"
#include "ps.hpp"

namespace alg {

sgd::sgd(paracel::Comm comm, 
	string hosts_dct_str, 
	string _input,
	string output,
	size_t _rounds,
	double _alpha,
	double _beta) :
	paracel::paralg(hosts_dct_str, comm, output, _rounds),
	input(_input),
	worker_id(comm.get_rank()),
	rounds(_rounds), 
	alpha(_alpha),
	beta(_beta) {}

sgd::~sgd() {}

void sgd::solve() {
}

} // namespace alg
