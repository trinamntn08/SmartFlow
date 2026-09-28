#include "tp_data/AbstractMemberFactory.h"

namespace tp_data
{

//##################################################################################################
AbstractMemberFactory::AbstractMemberFactory(const tp_utils::StringID& type, const std::string& extension, TPPixel color):
  m_type(type),
  m_extension(extension),
  m_color(color)
{

}

//##################################################################################################
AbstractMemberFactory::~AbstractMemberFactory() = default;

//##################################################################################################
const tp_utils::StringID& AbstractMemberFactory::type() const
{
  return m_type;
}

//##################################################################################################
const std::string& AbstractMemberFactory::extension() const
{
  return m_extension;
}

//##################################################################################################
TPPixel AbstractMemberFactory::color() const
{
  return m_color;
}

}
