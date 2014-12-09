/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */
#ifndef FILE_56636034_fc34_4f9e_4343_4111ca3b127d_HPP
#define FILE_56636034_fc34_4f9e_4343_4111ca3b127d_HPP

#include <sys/stat.h>
#include <glob.h>
#include <map>
#include <vector>
#include <numeric>
#include <boost/algorithm/string/regex.hpp>
#include "paracel_types.hpp"

namespace paracel {

typedef paracel::list_type< paracel::str_type > slst_type;

slst_type str_split(const paracel::str_type & str, const char sep) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s.size()) result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

slst_type str_split(paracel::str_type && str, const char sep) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s.size()) result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

slst_type str_split(const paracel::str_type & str, const paracel::str_type & seps) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find_first_of(seps, st);
    auto s = str.substr(st, en - st);
    if(s.size()) result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}


slst_type str_split(paracel::str_type && str, const paracel::str_type & seps) {
  slst_type result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find_first_of(seps, st);
    auto s = str.substr(st, en - st);
    if(s.size()) result.push_back(std::move(s));
    if(en == paracel::str_type::npos) break;
    st = en + 1;
  }
  return result;
}

slst_type str_split_by_word(const paracel::str_type & str, const paracel::str_type & seps) {
  slst_type result;
  boost::algorithm::split_regex(result, str, boost::regex(seps));
  if(result[result.size() - 1] == "") result.pop_back();
  return result;
}

slst_type str_split_by_word(paracel::str_type && str, const paracel::str_type & seps) {
  slst_type result;
  boost::algorithm::split_regex(result, std::move(str), boost::regex(seps));
  if(result[result.size() - 1] == "") result.pop_back();
  return result;
}

/*
slst_type str_split_by_word(const paracel::str_type & str, const paracel::str_type & seps) {
  slst_type result;
  auto last = str.begin();
  auto i = str.begin();
  for(; i != str.end(); ++i) {
    if(paracel::str_type(i, i + seps.size()) == seps) {
      result.push_back(paracel::str_type(last, i));
      last = i + seps.size();
    }
  }
  if(last != i) {
    result.push_back(paracel::str_type(last, i));
  }
  return result;
}

slst_type str_split_by_word(paracel::str_type && str, const paracel::str_type & seps) {
  slst_type result;
  auto last = str.begin();
  auto i = str.begin();
  for(; i != str.end(); ++i) {
    if(paracel::str_type(i, i + seps.size()) == seps) {
      result.push_back(paracel::str_type(last, i));
      last = i + seps.size();
    }
  }
  if(last != i) {
    result.push_back(paracel::str_type(last, i));
  }
  return result;
}
*/


paracel::str_type str_join(const slst_type & strlst, const paracel::str_type & seps) {
  paracel::str_type r;
  for(size_t i = 0; i < strlst.size() - 1; ++i) {
    r.append(strlst[i]);
    r.append(seps);
  }
  r.append(strlst[strlst.size() - 1]);
  return r;
}

paracel::str_type str_join(slst_type && strlst, const paracel::str_type & seps) {
  paracel::str_type r;
  for(size_t i = 0; i < strlst.size() - 1; ++i) {
    r.append(std::move(strlst[i]));
    r.append(seps);
  }
  r.append(std::move(strlst[strlst.size() - 1]));
  return r;
}

bool startswith(const paracel::str_type & str, const paracel::str_type & key) {
  return str.find(key) == 0;
}

bool endswith(const paracel::str_type & str, const std::string & key) {
  return str.rfind(key) == (str.length() - key.length());
}

// ['a', 'c', 'b', 'a', 'a', 'b'] -> [3, 2, 1]
paracel::list_type<int> 
sort_and_cnt(const paracel::list_type<paracel::str_type> & in) {
  paracel::list_type<int> r;
  std::map<paracel::str_type, int> m;
  for(auto & str : in) {
    if(m.find(str) == m.end()) {
      m[str] = 1;
    } else {
      m[str] += 1;
    }
  }
  for(auto & mp : m) {
    r.push_back(mp.second);
  }
  return r;
}

bool isfile(const paracel::str_type & f) {
  struct stat st;
  int r = stat(f.c_str(), &st);
  if(r != 0) {
    return false;
  }
  if(S_ISREG(st.st_mode)) {
    return true;
  }
  return false;
}

bool isdir(const paracel::str_type & d) {
  struct stat st;
  int r = stat(d.c_str(), &st);
  if(r != 0) {
    return false;
  }
  if(S_ISDIR(st.st_mode)) {
    return true;
  }
  return false;
}

paracel::list_type<paracel::str_type> 
paracel_glob(const paracel::str_type & pattern) {
  glob_t glob_res;
  paracel::list_type<paracel::str_type> lst;
  glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_res);
  for(size_t i = 0; i < glob_res.gl_pathc; ++i) {
    lst.push_back(std::string(glob_res.gl_pathv[i]));
  }
  globfree(&glob_res);
  return lst;
}

// expand a dictory name recursively
paracel::list_type<paracel::str_type> 
expand_dir_rec(const paracel::str_type & dname) {
 paracel::list_type<paracel::str_type> fl;
 auto dname_new = dname;
 if(!endswith(dname, "/")) {
   dname_new = dname + "/";
 }
 auto lst = paracel_glob(dname_new + "*");
 for(auto & name : lst) {
   if(isfile(name)) {
     fl.push_back(name);
   } else {
     auto tmp_lst = expand_dir_rec(name);
     fl.insert(fl.end(), tmp_lst.begin(), tmp_lst.end());
   }
 }
 return fl;
}

// fname can be 'demo.txt' or 'demo_dir' or 'demo_dir/*.csv'
paracel::list_type<paracel::str_type> 
expand(const paracel::str_type & fname) {
  paracel::list_type<paracel::str_type> fl;
  if(isfile(fname)) {
    fl.push_back(fname);
    return fl;
  } else if(isdir(fname)) {
    return expand_dir_rec(fname); 
  } else {
    // expect a reg exp: '/home/xunzhang/*.csv'
    return paracel_glob(fname);
  } 
}

// fname_lst can be ['demo.txt', 'demo_dir', 'demo_dir/*.txt', ...]
paracel::list_type<paracel::str_type> 
expand(const paracel::list_type<paracel::str_type> & fname_lst) {
  paracel::list_type<paracel::str_type> fl;
  for(auto & name : fname_lst) {
    auto tmp_lst = expand(name);
    fl.insert(fl.end(), tmp_lst.begin(), tmp_lst.end());
  }
  return fl;
}

inline double dot_product(const std::vector<double> & a, const std::vector<double> & b) {
  return std::inner_product(a.begin(), a.end(), b.begin(), 0.);
}


paracel::str_type todir(const paracel::str_type & f) {
  if(endswith(f, "/")) {
    return f;
  } 
  return f + "/";
}

} // namespace paracel
#endif
