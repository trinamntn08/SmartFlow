#pragma once

#include "tp_utils/StringID.h"

namespace tp_utils
{

//##################################################################################################
inline void hash_combine(uint64_t& seed, uint64_t value)
{
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

//##################################################################################################
inline void hash_combine(uint64_t& seed, const std::string& v)
{
  std::hash<std::string> hasher;
  hash_combine(seed, hasher(v));
}

//##################################################################################################
inline void hash_combine(uint64_t& seed, const StringID& v)
{
  hash_combine(seed, v.toString());
}

//##################################################################################################
template <typename T>
inline void hash_combine(uint64_t& seed, const T& v)
{
  std::hash<T> hasher;
  hash_combine(seed, hasher(v));
}

}
