/**
 * Copyright (c) 2013, Douban Inc. 
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
#ifndef FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP
#define FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP

#include <functional>
#include "paracel_types.hpp"

namespace paracel {

template <class T>
class loader {
public:
private:
  T filenames;
  Comm m_comm;
  std::function< paracel::list_type<paracel::str_type>(paracel::str_type) parser;
  paracel::str_type pattern = "fmap";
  bool mix = false;
};

} // namespace paracel

#endif
