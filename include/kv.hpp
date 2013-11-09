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
#ifndef FILE_0c75247e_03c0_5a81_3776_1d686062eb51_HPP
#define FILE_0c75247e_03c0_5a81_3776_1d686062eb51_HPP

//#include <map>
#include <unordered_map>
#include <vector>
#include <boost/optional/optional.hpp>
#include "paracel_types.hpp"

namespace paracel {

template <class K, class V>
struct kvs {

public:

  inline virtual ~kvs() {}
  
  inline bool contains(const K & k) { return (kvdct.find(k) != kvdct.end()); }
  
  void set(const K & k, const V & v) {
    kvdct[k] = v;
  }

  void set_multi(const paracel::dict_type<K, V> & kvdict) {
    for(auto & kv : kvdict) {
      set(kv.first, kv.second);
    }
  }

  /*
  // another version of get, more mem will be used
  paracel_list_type<V> get(const K & k) {
    parace_list_type tmp_val_list;
    if(contains(k)) {
      tmp_val_list.push_back(kvdct.at(k));
    } 
    return tmp_val_list; 
  }
  */

  boost::optional<V> get(const K & k) {
    if(contains(k)) {
      return boost::optional<V>(kvdct.at(k));
    } else { return boost::none; }
  }

  bool get(const K & k, V & v) {
    if(contains(k)) {
      v = kvdct.at(k);
      return true;
    } else { return false; }
  }

  paracel::list_type<V> get_multi(const paracel::list_type<K> & keylst) {
    paracel::list_type<V> valst;
    for(auto & key : keylst) {
      valst.push_back(key);
    }
    return valst;
  } 
  
  // tricky:
  // T is equal to V actually, but I want to apply SFINAE here
  // use a function template to cheat
  template <class T> 
  paracel::Enable_if<paracel::is_atomic<T>::value, bool>
  incr(const K & k, const T & delta) {
    if(!contains(k)) return false;
    kvdct[k] += delta;
    return true;
  }

  template <class T>
  paracel::Enable_if<paracel::is_seqic<T>::value, bool>
  incr(const K & k, const T & delta) {
    if(!contains(k)) return false;
    if(kvdct[k].size() != delta.size()) return false;
    for(int i = 0; i < delta.size(); ++i) {
      kvdct[k][i] += delta[i];
    }
    return true;
  }

  boost::optional<std::pair<V, paracel::hash_return_type> >
  gets(const K & k) {
    if(auto v = get(k)) {
      std::pair<V, paracel::hash_return_type> ret(*v, hfunc(*v));
      return boost::optional<std::pair<V, paracel::hash_return_type> >(ret);
    } else {
      return boost::none;
    }
  }

  bool cas(const K & k, const V & v, const paracel::hash_return_type & uniq) {
    if(auto r = gets(k)) {
      if(uniq == (*r).second) {
        set(k, v);
	return true;
      } else { return false; }
    } else {
      kvdct[k] = v;
    }
  }

  bool del(const K & k) {
    if(!contains(k)) return false;
    kvdct.erase(k);
    return true;
  }
  
  void clean() {
    kvdct.clear();
  }
  
  paracel::dict_type<K, V> getall() {
    return kvdct;
  }

private:
  paracel::dict_type<K, V> kvdct;
  paracel::hash_type<V> hfunc;
};

} // namespace paracel
#endif
