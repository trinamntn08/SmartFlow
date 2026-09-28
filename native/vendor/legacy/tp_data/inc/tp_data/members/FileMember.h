#pragma once

#include "tp_data/AbstractMemberFactory.h"

namespace tp_data
{

//##################################################################################################
class FileMember : public tp_data::AbstractMember
{
public:
  //################################################################################################
  FileMember(const tp_utils::StringID& name=tp_utils::StringID(), const std::string& data_=std::string());

  //################################################################################################
  ~FileMember();

  //################################################################################################
  static FileMember* fromData(std::string& error, const std::string& data);

  //################################################################################################
  std::string toData() const;

  //################################################################################################
  void copyData(const FileMember& other);

  static const std::string extension;
  std::string data;
};

//##################################################################################################
using FileMemberFactory = tp_data::MultiDataMemberFactoryTemplate<FileMember, fileSID>;

}
