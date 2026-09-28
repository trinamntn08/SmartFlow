#pragma once

#include "tp_data/AbstractMemberFactory.h"

namespace tp_data
{

//##################################################################################################
class StringMember : public tp_data::AbstractMember
{
public:
  //################################################################################################
  StringMember(const tp_utils::StringID& name=tp_utils::StringID(), const std::string& data_=std::string());

  //################################################################################################
  ~StringMember();

  //################################################################################################
  static StringMember* fromData(std::string& error, const std::string& data);

  //################################################################################################
  std::string toData() const;

  //################################################################################################
  void copyData(const StringMember& other);

  static const std::string extension;
  std::string data;
};

//##################################################################################################
using StringMemberFactory = tp_data::MultiDataMemberFactoryTemplate<StringMember, stringSID>;

}
