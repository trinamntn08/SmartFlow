#include "tp_data/AbstractMember.h"

#include "tp_utils/TimeUtils.h"

namespace tp_data
{

//##################################################################################################
AbstractMember::AbstractMember(const tp_utils::StringID& name, const tp_utils::StringID& type):
  m_name(name),
  m_type(type),
  m_timestampMS(tp_utils::currentTimeMS())
{

}

//##################################################################################################
AbstractMember::~AbstractMember() = default;

//##################################################################################################
const tp_utils::StringID& AbstractMember::name() const
{
  return m_name;
}

//##################################################################################################
void AbstractMember::setName(const tp_utils::StringID& name)
{
  m_name = name;
}

//##################################################################################################
const tp_utils::StringID& AbstractMember::type() const
{
  return m_type;
}

//##################################################################################################
int64_t AbstractMember::timestampMS() const
{
  return m_timestampMS;
}

//##################################################################################################
void AbstractMember::setTimestampMS(int64_t timestampMS)
{
  m_timestampMS = timestampMS;
}

//##################################################################################################
std::string AbstractMember::toData() const
{
  return "AbstractMember::toData";
}

}
