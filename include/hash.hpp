/**
 * Copyright (c) 2013, Douban Inc. 
 *   All rights reserved. 
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone http://code.dapps.douban.com/wuhong/paracel.git
 *
 * Authors
 *   Changsheng Jiang <jiangzuoyan@gmail.com>
 *   Hong Wu <xunzhangthu@gmail.com>
 *
 */
#ifndef FILE_45bb1820_07f5_4c07_8163_35d6870a4475_H
#define FILE_45bb1820_07f5_4c07_8163_35d6870a4475_H
#include <utility>
#include <functional>
#include <sys/types.h>

#include <cstdint>
#include <endian.h>

#include <vector>
#include <string>
#include <sstream>
namespace douban {
/**
 * @addtogroup utility
 * @{
 */

template <class T>
struct hash {
  size_t operator()(const T &t) const {
    return std::hash<T>()(t);
  }
};

struct meta_hash {
  template <class T>
  size_t operator()(const T &t) const {
    return douban::hash<T>()(t);
  }
};

inline uint64_t hash_value_combine(const uint64_t x, const uint64_t y) {
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (x ^ y) * kMul;
  a ^= (a >> 47);
  uint64_t b = (y ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

/**
 * Compute hash of @p v with seed @p seed.
 *
 * @return computed hash value
 */
template <class T>
inline size_t hash_combine(size_t seed, const T& v) {
  hash<T> hh;
  auto h = hh(v);
  return hash_value_combine(h, seed);
}

template <class L>
struct hash< std::vector<L> > {
  size_t operator()(const std::vector<L> & vec) const {
    std::string buff;
    for(auto & item : vec) {
      std::ostringstream cvt;
      cvt << item;
      buff.append(cvt.str());
    }
    hash<std::string> hl;
    return hl(buff);
  }
};

template <class L, class R>
struct hash< std::pair<L, R> > {
  size_t operator()(const std::pair<L, R> &n) const {
    hash<L> hl;
    return hash_combine(hl(n.first), n.second);
  }
};

size_t hash_bytes(const void * ptr, size_t len, size_t seed);

#if __WORDSIZE == 64
#define FNV1_INIT ((uint64_t)0xcbf29ce484222325ULL)
#define FNV1A_INIT ((uint64_t)0xcbf29ce484222325ULL)
#else
#define FNV1_INIT ((uint32_t)0x811c9dc5)
#define FNV1A_INIT ((uint32_t)0x811c9dc5)
#endif

size_t fnv_hash_bytes(const void * ptr, size_t len, size_t seed);
uint32_t fnv_hash_bytes_32(const void * ptr, size_t len, uint32_t seed);
uint64_t fnv_hash_bytes_64(const void * ptr, size_t len, uint64_t seed);

size_t spooky_hash_bytes(const void * ptr, size_t len, size_t seed);
uint32_t spooky_hash_bytes_32(const void *ptr, size_t len, uint32_t seed);
uint64_t spooky_hash_bytes_64(const void *ptr, size_t len, uint64_t seed);
void spooky_hash_bytes_128(const void *ptr, size_t len,
                           uint64_t *in_seed_out_hash0,
                           uint64_t *in_seed_out_hash1);

uint64_t city_hash_bytes_64(const void * ptr, size_t len);
uint64_t city_hash_bytes_64(const void * ptr, size_t len, uint64_t seed);
uint64_t city_hash_bytes_64(const void *ptr, size_t len, uint64_t seed0, uint64_t seed1);
void city_hash_bytes_128_noseed(const void *ptr, size_t len, uint64_t *h0, uint64_t *h1);
void city_hash_bytes_128(const void *ptr, size_t len,
                         uint64_t *in_seed_out_hash0,
                         uint64_t *in_seed_out_hash1);

inline size_t city_hash_bytes(const void * ptr, size_t len, size_t seed) {
  return city_hash_bytes_64(ptr, len, seed);
}

inline size_t city_hash_bytes(const void * ptr, size_t len) {
  return city_hash_bytes_64(ptr, len);
}

/** @} */
} // namespace douban
#endif
