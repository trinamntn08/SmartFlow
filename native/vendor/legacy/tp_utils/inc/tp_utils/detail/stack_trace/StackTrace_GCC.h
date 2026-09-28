#ifndef tp_utils_StackTrace_BasicImpl_h
#define tp_utils_StackTrace_BasicImpl_h

#include "tp_utils/StackTrace.h"
#include "tp_utils/detail/stack_trace/GCCStackTrace.h" // IWYU pragma: keep

namespace tp_utils
{

#define COUNT_MAX_LEVELS 12

//##################################################################################################
struct StackTrace::Private
{
  std::array<void*, COUNT_MAX_LEVELS> array{tpMakeArray<void*, COUNT_MAX_LEVELS>(nullptr)};
  size_t size;
  size_t hash;
};

//##################################################################################################
StackTrace::StackTrace():
  d(new Private)
{
  d->size = size_t(std::max(0, backtrace(d->array.data(), COUNT_MAX_LEVELS)));

  d->hash = std::hash<void*>()(nullptr);
  for(size_t c=0; c<d->size; c++)
    d->hash ^= std::hash<void*>()(d->array.at(c)) + 0x9e3779b9 + (d->hash<<6) + (d->hash>>2);
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
  return d->size;
}

//##################################################################################################
std::vector<std::string> StackTrace::frames() const
{
  //Convert the backtrace to strings
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(d->array.data(), int(d->size)), &free);

  std::vector<std::string> results;
  for(size_t i = 0; i < d->size; i++)
    results.push_back(strings.get()[i]);

  return results;
}

//##################################################################################################
bool StackTrace::operator==(const StackTrace& other) const
{
  return d->hash == other.d->hash && d->array == other.d->array;
}

//##################################################################################################
bool StackTrace::operator!=(const StackTrace& other) const
{
  return d->hash != other.d->hash || d->array != other.d->array;
}

//##################################################################################################
size_t StackTrace::hash() const
{
  return d->hash;
}

}

#endif
