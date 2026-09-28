#pragma once

#include <random>

namespace lib_platform
{

//##################################################################################################
//! We use random_device to seed uuid creation but random_device seems to be predictable on some platforms... so
class RandomDevice : public std::random_device
{
public:
  RandomDevice();
  result_type operator()();

private:
  static result_type initMT();
  static result_type initSeq();
  result_type m_seed;
};

}
