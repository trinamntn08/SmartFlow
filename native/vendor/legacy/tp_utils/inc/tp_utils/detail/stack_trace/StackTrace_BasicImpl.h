#ifndef tp_utils_StackTrace_BasicImpl_h
#define tp_utils_StackTrace_BasicImpl_h

#include "tp_utils/StackTrace.h"

namespace tp_utils
{

//##################################################################################################
struct StackTrace::Private
{
  std::vector<std::string> frames;
  size_t hash;
};

//##################################################################################################
StackTrace::StackTrace():
  d(new Private)
{
  d->frames = stackTraceFrames();

  d->hash = std::hash<void*>()(nullptr);
  for(const auto& frame : d->frames)
    d->hash ^= std::hash<std::string>()(frame) + 0x9e3779b9 + (d->hash<<6) + (d->hash>>2);
}

//##################################################################################################
StackTrace::StackTrace(const StackTrace& other):
  d(new Private)
{
  (*d) = (*other.d);
}

//##################################################################################################
StackTrace::StackTrace(StackTrace&& other) noexcept
{
  d = other.d;
  other.d = nullptr;
}

//##################################################################################################
StackTrace::~StackTrace()
{
  delete d;
}

//##################################################################################################
StackTrace& StackTrace::operator=(const StackTrace& other)
{
  if(&other == this)
    return *this;

  (*d) = (*other.d);
  return *this;
}

//##################################################################################################
StackTrace& StackTrace::operator=(StackTrace&& other) noexcept
{
  if(&other == this)
    return *this;

  delete d;
  d = other.d;
  other.d = nullptr;

  return *this;
}

//##################################################################################################
size_t StackTrace::size() const
{
  return d->frames.size();
}

//##################################################################################################
std::vector<std::string> StackTrace::frames() const
{
  return d->frames;
}

//##################################################################################################
bool StackTrace::operator==(const StackTrace& other) const
{
  return d->frames == other.d->frames;
}

//##################################################################################################
bool StackTrace::operator!=(const StackTrace& other) const
{
  return d->frames != other.d->frames;
}

//##################################################################################################
size_t StackTrace::hash() const
{
  return d->hash;
}

}

#endif
