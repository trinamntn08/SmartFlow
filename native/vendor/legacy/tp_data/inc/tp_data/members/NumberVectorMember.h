#pragma once

#include "tp_data/AbstractMemberFactory.h"

#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>

namespace tp_data
{
//##################################################################################################
struct NumberVectorMemberExtension
{
  static const std::string extension;
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class NumberVectorMember : public tp_data::AbstractMember, public NumberVectorMemberExtension
{
public:
  //################################################################################################
  NumberVectorMember(const tp_utils::StringID& name=tp_utils::StringID()):
    AbstractMember(name, type_())
  {

  }

  //################################################################################################
  static NumberVectorMember* fromData(std::string& error, const std::string& data)
  {
    TP_UNUSED(error);
    auto member = new NumberVectorMember<T, type_>();
    std::istringstream stream(data);
    member->data = std::vector<T>(std::istream_iterator<T>(stream), std::istream_iterator<T>());
    return member;
  }

  //################################################################################################
  std::string toData() const
  {
    if(data.empty())
      return "";

    std::ostringstream stream;
    std::copy(data.begin(), data.end() - 1, std::ostream_iterator<T>(stream, " "));
    stream << data.back();
    return stream.str();
  }

  //################################################################################################
  void copyData(const NumberVectorMember<T, type_>& other)
  {
    data = other.data;
  }

  std::vector<T> data;
};

//##################################################################################################
// Assumes the existence of corresponding StringID functions for vector types.
using    IntVectorMember = tp_data::NumberVectorMember<   int,    intVectorSID>;
using  SizeTVectorMember = tp_data::NumberVectorMember<size_t,  sizeTVectorSID>;
using  FloatVectorMember = tp_data::NumberVectorMember< float,  floatVectorSID>;
using DoubleVectorMember = tp_data::NumberVectorMember<double, doubleVectorSID>;

//##################################################################################################
using    IntVectorMemberFactory = tp_data::MultiDataMemberFactoryTemplate<   IntVectorMember,    intVectorSID>;
using  SizeTVectorMemberFactory = tp_data::MultiDataMemberFactoryTemplate< SizeTVectorMember,  sizeTVectorSID>;
using  FloatVectorMemberFactory = tp_data::MultiDataMemberFactoryTemplate< FloatVectorMember,  floatVectorSID>;
using DoubleVectorMemberFactory = tp_data::MultiDataMemberFactoryTemplate<DoubleVectorMember, doubleVectorSID>;

}
