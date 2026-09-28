#pragma once

#include "tp_data/AbstractMemberFactory.h"

#include <sstream>

namespace tp_data
{
//##################################################################################################
struct NumberMemberExtension
{
  static const std::string extension;
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class NumberMember : public tp_data::AbstractMember, public NumberMemberExtension
{
public:
  //################################################################################################
  NumberMember(const tp_utils::StringID& name=tp_utils::StringID()):
    AbstractMember(name, type_())
  {

  }

  //################################################################################################
  static NumberMember* fromData(std::string& error, const std::string& data)
  {
    TP_UNUSED(error);
    auto member = new NumberMember<T, type_>();
    std::istringstream(data) >> member->data;
    return member;
  }

  //################################################################################################
  std::string toData() const
  {
    return std::to_string(data);
  }

  //################################################################################################
  void copyData(const NumberMember<T, type_>& other)
  {
    data = other.data;
  }

  T data;
};

//##################################################################################################
using    IntMember = tp_data::NumberMember<   int,    intSID>;
using  SizeTMember = tp_data::NumberMember<size_t,  sizeTSID>;
using  FloatMember = tp_data::NumberMember< float,  floatSID>;
using DoubleMember = tp_data::NumberMember<double, doubleSID>;

//##################################################################################################
using    IntMemberFactory = tp_data::MultiDataMemberFactoryTemplate<   IntMember,    intSID>;
using  SizeTMemberFactory = tp_data::MultiDataMemberFactoryTemplate< SizeTMember,  sizeTSID>;
using  FloatMemberFactory = tp_data::MultiDataMemberFactoryTemplate< FloatMember,  floatSID>;
using DoubleMemberFactory = tp_data::MultiDataMemberFactoryTemplate<DoubleMember, doubleSID>;

}
