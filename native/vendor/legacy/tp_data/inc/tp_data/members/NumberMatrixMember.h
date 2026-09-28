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
struct NumberMatrixMemberExtension
{
  static const std::string extension;
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class NumberMatrixMember : public tp_data::AbstractMember, public NumberMatrixMemberExtension
{
public:
  //################################################################################################
  NumberMatrixMember(const tp_utils::StringID& name=tp_utils::StringID()):
    AbstractMember(name, type_())
  {
  }

  //################################################################################################
  NumberMatrixMember(const tp_utils::StringID& name, size_t rows_, size_t cols_):
    AbstractMember(name, type_()),
    rows(rows_),
    cols(cols_)
  {
    data.resize(rows * cols, T(0));
  }

  //################################################################################################
  static NumberMatrixMember* fromData(std::string& error, const std::string& data)
  {
    TP_UNUSED(error);
    auto member = new NumberMatrixMember<T, type_>();
    std::istringstream stream(data);
    member->data = std::vector<T>(std::istream_iterator<T>(stream), std::istream_iterator<T>());
    member->rows = member->data.size();
    member->cols = member->rows>0 ? 1 : 0;
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
  T& at(size_t r, size_t c)
  {
    return data[c * rows + r];
  }

  //################################################################################################
  const T& at(size_t r, size_t c) const
  {
    return data[c * rows + r];
  }

  //################################################################################################
  void copyData(const NumberMatrixMember<T, type_>& other)
  {
    data = other.data;
    rows = other.rows;
    cols = other.cols;
  }

  // The matrix is stored in a flattened, 1D vector for contiguous memory.
  std::vector<T> data;
  size_t rows{0};
  size_t cols{0};
};

//##################################################################################################
// Assumes the existence of corresponding StringID functions for matrix types.
using    IntMatrixMember = tp_data::NumberMatrixMember<   int,    intMatrixSID>;
using  SizeTMatrixMember = tp_data::NumberMatrixMember<size_t,  sizeTMatrixSID>;
using  FloatMatrixMember = tp_data::NumberMatrixMember< float,  floatMatrixSID>;
using DoubleMatrixMember = tp_data::NumberMatrixMember<double, doubleMatrixSID>;

//##################################################################################################
using    IntMatrixMemberFactory = tp_data::MultiDataMemberFactoryTemplate<   IntMatrixMember,    intMatrixSID>;
using  SizeTMatrixMemberFactory = tp_data::MultiDataMemberFactoryTemplate< SizeTMatrixMember,  sizeTMatrixSID>;
using  FloatMatrixMemberFactory = tp_data::MultiDataMemberFactoryTemplate< FloatMatrixMember,  floatMatrixSID>;
using DoubleMatrixMemberFactory = tp_data::MultiDataMemberFactoryTemplate<DoubleMatrixMember, doubleMatrixSID>;

}
