#include "tp_data/members/FileMember.h"

namespace tp_data
{
const std::string FileMember::extension{"txt"};

//##################################################################################################
FileMember::FileMember(const tp_utils::StringID& name, const std::string& data_):
  AbstractMember(name, stringSID())
{
  data = data_;
}

//##################################################################################################
FileMember::~FileMember() = default;

//##################################################################################################
FileMember* FileMember::fromData(std::string& error, const std::string& data)
{
  TP_UNUSED(error);
  auto member = new FileMember();
  member->data = data;
  return member;
}

//##################################################################################################
std::string FileMember::toData() const
{
  return data;
}

//##################################################################################################
void FileMember::copyData(const FileMember& other)
{
  data = other.data;
}

}

