#include "tp_pipeline/ComplexObject.h"

namespace tp_pipeline
{

//##################################################################################################
ComplexObject::ComplexObject(tp_utils::StringID type):
  m_type(std::move(type))
{

}

//##################################################################################################
ComplexObject::~ComplexObject() = default;

//##################################################################################################
tp_utils::StringID ComplexObject::type()const
{
  return m_type;
}

}
