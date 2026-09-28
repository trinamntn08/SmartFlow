#pragma once

#include "tp_data/AbstractMemberFactory.h"

namespace tp_data
{

//##################################################################################################
class StringIDVectorMember : public tp_data::AbstractMember
{
public:
  //################################################################################################
  StringIDVectorMember(const tp_utils::StringID& name=tp_utils::StringID(), const std::vector<tp_utils::StringID>& data_=std::vector<tp_utils::StringID>());

  //################################################################################################
  ~StringIDVectorMember();

  //################################################################################################
  static StringIDVectorMember* fromData(std::string& error, const std::string& data);

  //################################################################################################
  std::string toData() const;

  //################################################################################################
  void copyData(const StringIDVectorMember& other);

  static const std::string extension;
  std::vector<tp_utils::StringID> data;
};

//##################################################################################################
using StringIDVectorMemberFactory = tp_data::MultiDataMemberFactoryTemplate<StringIDVectorMember, stringIDVectorSID>;

}
