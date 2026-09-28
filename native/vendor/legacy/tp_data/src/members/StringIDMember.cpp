#include "tp_data/members/StringIDMember.h"

namespace tp_data
{
const std::string StringIDMember::extension{"txt"};

//##################################################################################################
StringIDMember::StringIDMember(const tp_utils::StringID& name, const tp_utils::StringID& data_):
  AbstractMember(name, stringIDSID())
{
  data = data_;
}

//##################################################################################################
StringIDMember::~StringIDMember() = default;

//##################################################################################################
StringIDMember* StringIDMember::fromData(std::string& error, const std::string& data)
{
  TP_UNUSED(error);
  auto member = new StringIDMember();
  member->data = data;
  return member;
}

//##################################################################################################
std::string StringIDMember::toData() const
{
  return data.toString();
}

//##################################################################################################
void StringIDMember::copyData(const StringIDMember& other)
{
  data = other.data;
}

}

