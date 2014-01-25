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
#ifndef FILE_0f372fdb_d853_338a_95b6_a3c1f4b9fc30_HPP
#define FILE_0f372fdb_d853_338a_95b6_a3c1f4b9fc30_HPP

#include <functional>

#include "paracel_types.hpp"
#include "utils/func_utility.hpp"
#include "packer.hpp"

namespace paracel {

using update_result = std::function<paracel::str_type(paracel::str_type, paracel::str_type)>;

/*
template <class F>
struct update_functor {
  update_result operator()(F func, paracel::str_type s_val, paracel::str_type s_delta) {
    paracel::packer<decltype(val)> pk1;
    paracel::packer<decltype()>
  };
};
*/

template <class F>
update_result update_proxy(F & func) {
  typedef paracel::f_traits<decltype(func)> traits;
  typename traits::result_type result;
  typename traits::template args<0>::type val;
  typename traits::template args<1>::type delta;
  update_result update_lambda = [&] (paracel::str_type s_val, paracel::str_type s_delta) {
    paracel::packer<decltype(val)> pk1;
    paracel::packer<decltype(delta)> pk2;
    auto tmp = func(pk1.unpack(s_val), pk2.unpack(s_delta));
    paracel::packer<decltype(result)> pk3(tmp);
    paracel::str_type r;
    pk3.pack(r);
    return r;
  };
  return update_lambda;
}

} // namespace paracel

#endif
