/**
 * Copyright (c) 2013, Douban Inc. 
 *   All rights reserved. 
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */
#ifndef FILE__149f02d6_0fec_56ac_97f4_a30cea847471__HPP
#define FILE__149f02d6_0fec_56ac_97f4_a30cea847471__HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <string>
#include <functional>

#include "hash.hpp"

namespace paracel {

template <class T>
struct is_atomic : std::false_type {};

template <class T>
struct is_seqic : std::false_type {};

#define PARACEL_REGISTER_ATOM(T, Dtype)		\
  template <>					\
  struct is_atomic<T> : std::true_type {	\
    static std::string datatype() {		\
      return Dtype;				\
    }						\
  }						\

#define PARACEL_REGISTER_SEQ(T, Dtype)		\
  template <>					\
  struct is_seqic<T> : std::true_type {	        \
    static std::string datatype() {		\
      return Dtype;				\
    }						\
  }						\

// paracel only support a subsets of std::atomic
// used for incr op
PARACEL_REGISTER_ATOM(int, "int");
PARACEL_REGISTER_ATOM(long, "long");
PARACEL_REGISTER_ATOM(short, "short");
PARACEL_REGISTER_ATOM(float, "float");
PARACEL_REGISTER_ATOM(double, "double");
PARACEL_REGISTER_ATOM(unsigned int, "unsigned int");
PARACEL_REGISTER_ATOM(unsigned short, "unsigned short");
PARACEL_REGISTER_ATOM(unsigned long, "unsigned long");

// paracel only supported continuous container
// used for incr op
PARACEL_REGISTER_SEQ(std::vector<int>, "std::vector<int>");
PARACEL_REGISTER_SEQ(std::vector<long>, "std::vector<long>");
PARACEL_REGISTER_SEQ(std::vector<short>, "std::vector<short>");
PARACEL_REGISTER_SEQ(std::vector<float>, "std::vector<float>");
PARACEL_REGISTER_SEQ(std::vector<double>, "std::vector<double>");
PARACEL_REGISTER_SEQ(std::vector<unsigned int>, "std::vector<unsigned int>");
PARACEL_REGISTER_SEQ(std::vector<unsigned long>, "std::vector<unsigned long>");
PARACEL_REGISTER_SEQ(std::vector<unsigned short>, "std::vector<unsigned short>");

using str_type = std::string;

using hash_return_type = size_t;

template <class T>
using list_type = std::vector<T>;

// hash function not support std::vector<bool> like which is supported in std::hash
// but support std::vector<int>...
/*
template <class T>
using hash_type = std::hash<T>;
*/
template <class T>
using hash_type = douban::hash<T>;

template <class K, class V>
using dict_type = std::unordered_map<K, V>;

template <bool Cond, class T = void>
using Enable_if = typename std::enable_if<Cond, T>::type;

template <bool Cond, class T = void>
using Disable_if = typename std::enable_if<!Cond, T>::type;

} // namespace paracel

#endif
