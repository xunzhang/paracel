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
#ifndef FILE_91c90a2b_6722_96c4_8e44_c75c4ff88d51_HPP 
#define FILE_91c90a2b_6722_96c4_8e44_c75c4ff88d51_HPP

#include <sstream>
#include <iostream>
#include <string>

#include <msgpack.hpp>
#include <msgpack/type/tr1/unordered_map.hpp>

#include "paracel_types.hpp"

namespace paracel {

template <class T = paracel::str_type>
struct packer {
public:

  // for unpack usage
  packer() {}

  packer(T v) : val(v) {}

  void pack(msgpack::sbuffer & sbuf) {
    msgpack::pack(&sbuf, val);
  }

  void pack(std::string & s) {
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, val);
    std::ostringstream oss;
    oss << sbuf.size();
    oss.write(sbuf.data(), sbuf.size());
    s = std::string(oss.str());
  }

  T unpack(const msgpack::sbuffer & sbuf) {
    T r;
    msgpack::unpacked msg;
    msgpack::unpack(&msg, sbuf.data(), sbuf.size());
    auto obj = msg.get();
    obj.convert(&r);
    return r;
  }

  T unpack(const std::string & s) {
    T r;
    msgpack::unpacked msg;
    std::istringstream iss(s);
    int sz;
    paracel::list_type<char> buf;
    iss >> sz;
    buf.resize(sz);
    iss.read(&buf[0], sz);
    msgpack::unpack(&msg, &buf[0], sz);
    auto obj = msg.get();
    obj.convert(&r);
    return r;
  }

private:
  T val;
};

} // namespace paracel
#endif
