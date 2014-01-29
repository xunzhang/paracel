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
using filter_result = std::function<bool(paracel::str_type, paracel::str_type)>;

template <class F>
filter_result filter_proxy(F & func) {
  typedef paracel::f_traits<decltype(func)> traits;
  //typename traits::result_type result;
  //typename traits::template args<0>::type key;
  //static_assert(std::is_same<bool, traits::result_type>::value, "filter function in paracel must return bool.");
  //static_assert(std::is_same<paracel::str_type, key>::value, "key type of filter function in paracel must std::string.");
  typename traits::template args<1>::type val;
  filter_result filter_lambda = [&] (paracel::str_type s_key, paracel::str_type s_val) {
    paracel::packer<> pk1;
    paracel::packer<decltype(val)> pk2;
    auto p2 = pk2.unpack(s_val);
    return func(s_key, p2); // bool
  };
  return filter_lambda;
}

template <class F>
update_result update_proxy(F & func) { // '&' is important
  typedef paracel::f_traits<decltype(func)> traits;
  typename traits::result_type result;
  typename traits::template args<0>::type val;
  typename traits::template args<1>::type delta;
  update_result update_lambda = [&] (paracel::str_type s_val, paracel::str_type s_delta) {
    paracel::packer<decltype(val)> pk1;
    paracel::packer<decltype(delta)> pk2;
    auto p1 = pk1.unpack(s_val);
    auto p2 = pk2.unpack(s_delta);
    auto tmp = func(p1, p2);
    paracel::packer<decltype(result)> pk3(tmp);
    paracel::str_type r;
    pk3.pack(r);
    return r;
  };
  return update_lambda;
}

} // namespace paracel

#endif
