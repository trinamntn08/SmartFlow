#pragma once

#include "tp_data/Globals.h"

#include <memory>

namespace tp_data
{
class AbstractMember;

//##################################################################################################
//! This holds a collection of data objects.
class TP_DATA_SHARED_EXPORT Collection
{
  TP_NONCOPYABLE(Collection);
  TP_DQ;
public:
  //################################################################################################
  Collection();

  //################################################################################################
  ~Collection();

  //################################################################################################
  const std::string& name() const;

  //################################################################################################
  void setName(const std::string& name);

  //################################################################################################
  //! When this data collection was created.
  /*!
  This will be initialized to the current time but factories may change this.
  \return The timestamp of this collection.
  */
  int64_t timestampMS()const;

  //################################################################################################
  //! set the timestamp of this collection.
  void setTimestampMS(int64_t timestampMS);

  //################################################################################################
  //! This takes ownership.
  void addMember(const std::shared_ptr<AbstractMember>& member);

  //################################################################################################
  const std::vector<std::shared_ptr<AbstractMember>>& members() const;

  //################################################################################################
  //! Find an member.
  /*!
  \note The Collection owns the returned member.
  \param name The unique name of the member to find.
  \returns A pointer to the member or nullptr.
  */
  const std::shared_ptr<AbstractMember>& member(const tp_utils::StringID& name) const;

  //################################################################################################
  template<typename T>
  void memberCast(const tp_utils::StringID& name, T*& member_) const
  {
    member_ = dynamic_cast<T*>(member(name).get());
  }  

  //################################################################################################
  template<typename T>
  T* memberCast(const tp_utils::StringID& name) const
  {
    return dynamic_cast<T*>(member(name).get());
  }

  //################################################################################################
  template<typename T>
  void memberCast(const std::function<void(const T&)> closure) const
  {
    for(const auto& member : members())
      if(auto m = dynamic_cast<T*>(member.get()); m)
        closure(*m);
  }

  //################################################################################################
  void clear();
};

}
