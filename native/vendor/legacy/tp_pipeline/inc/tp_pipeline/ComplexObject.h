#pragma once

#include "tp_pipeline/Globals.h"

#include "tp_utils/JSONUtils.h" // IWYU pragma: keep

namespace tp_pipeline
{

//##################################################################################################
class TP_PIPELINE_SHARED_EXPORT ComplexObject
{
  TP_NONCOPYABLE(ComplexObject);
  tp_utils::StringID m_type;
public:
  //################################################################################################
  ComplexObject(tp_utils::StringID type);

  //################################################################################################
  virtual ~ComplexObject();

  //################################################################################################
  tp_utils::StringID type()const;

  //################################################################################################
  virtual void saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const=0;

  //################################################################################################
  virtual ComplexObject* clone()const=0;
};

}
