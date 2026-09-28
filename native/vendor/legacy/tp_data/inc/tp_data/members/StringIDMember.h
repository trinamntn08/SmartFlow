#pragma once

#include "tp_data/AbstractMemberFactory.h"

namespace tp_data
{

//##################################################################################################
class StringIDMember : public tp_data::AbstractMember
{
public:
  //################################################################################################
  StringIDMember(const tp_utils::StringID& name=tp_utils::StringID(), const tp_utils::StringID& data_=tp_utils::StringID());

  //################################################################################################
  ~StringIDMember();

  //################################################################################################
  static StringIDMember* fromData(std::string& error, const std::string& data);

  //################################################################################################
  std::string toData() const;

  //################################################################################################
  void copyData(const StringIDMember& other);

  static const std::string extension;
  tp_utils::StringID data;
};

//##################################################################################################
using StringIDMemberFactory = tp_data::MultiDataMemberFactoryTemplate<StringIDMember, stringIDSID>;

}
