#include "tp_data/Collection.h"
#include "tp_data/AbstractMember.h"

#include "tp_utils/TimeUtils.h"

namespace tp_data
{

//##################################################################################################
struct Collection::Private
{
  TP_NONCOPYABLE(Private);
  std::string name;
  int64_t timestampMS{tp_utils::currentTimeMS()};
  std::vector<std::shared_ptr<AbstractMember>> members;

  //################################################################################################
  Private()=default;
};

//##################################################################################################
Collection::Collection():
  d(new Private())
{

}

//##################################################################################################
Collection::~Collection()
{
  delete d;
}

//##################################################################################################
const std::string& Collection::name() const
{
  return d->name;
}

//##################################################################################################
void Collection::setName(const std::string& name)
{
  d->name = name;
}

//##################################################################################################
int64_t Collection::timestampMS()const
{
  return d->timestampMS;
}

//##################################################################################################
void Collection::setTimestampMS(int64_t timestampMS)
{
  d->timestampMS = timestampMS;
}

//##################################################################################################
void Collection::addMember(const std::shared_ptr<AbstractMember>& member)
{
  if(!member)
    return;

  d->members.push_back(member);
}

//##################################################################################################
const std::vector<std::shared_ptr<AbstractMember>>& Collection::members() const
{
  return d->members;
}

//##################################################################################################
const std::shared_ptr<AbstractMember>& Collection::member(const tp_utils::StringID& name) const
{
  static thread_local std::shared_ptr<tp_data::AbstractMember> n;

  for(const auto& member : d->members)
  {
    if(member->name() == name)
      return member;
  }

  return n;
}

//##################################################################################################
void Collection::clear()
{
  d->members.clear();
}

}
