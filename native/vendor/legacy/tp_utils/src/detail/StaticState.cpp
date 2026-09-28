#include "tp_utils/detail/StaticState.h"

#include <cassert>

namespace tp_utils
{

//##################################################################################################
std::shared_ptr<StaticState> StaticState::instance()
{
  static std::shared_ptr<StaticState> instance{std::make_shared<StaticState>()};
  return instance;
}

}

