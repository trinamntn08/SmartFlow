#ifndef lib_platform_OSX_h
#define lib_platform_OSX_h

#include <algorithm>

#include "lib_platform/RandomDevice.h"

#define TP_ENUM_HASH(c)

namespace std
{

//##################################################################################################
//This is used in some external lib header files
template< class RandomIt, class RandomFunc >
void random_shuffle( RandomIt first, RandomIt last, RandomFunc&&)
{
  std::shuffle(first, last, []() {
    std::mt19937::result_type seeds[std::mt19937::state_size];
    lib_platform::RandomDevice device;
    std::uniform_int_distribution<typename std::mt19937::result_type> dist;
    std::generate(std::begin(seeds), std::end(seeds), [&] { return dist(device); });
    std::seed_seq seq(std::begin(seeds), std::end(seeds));
    return std::mt19937(seq);
  }());
}

//##################################################################################################
template< class RandomIt >
void random_shuffle( RandomIt first, RandomIt last)
{
  std::shuffle(first, last, []() {
    std::mt19937::result_type seeds[std::mt19937::state_size];
    lib_platform::RandomDevice device;
    std::uniform_int_distribution<typename std::mt19937::result_type> dist;
    std::generate(std::begin(seeds), std::end(seeds), [&] { return dist(device); });
    std::seed_seq seq(std::begin(seeds), std::end(seeds));
    return std::mt19937(seq);
  }());
}

}

#endif
