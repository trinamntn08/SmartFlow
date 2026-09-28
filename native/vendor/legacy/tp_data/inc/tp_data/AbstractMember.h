#pragma once

#include "tp_data/Globals.h" // IWYU pragma: keep

#include <memory>

namespace tp_data
{

//##################################################################################################
//! The base class for members of data collections.
/*!
This should be subclassed for each type of data that we need to store in a data collection. This
allows the data collection to store a wide range of data without poluting tp_data with dependencies.
*/
class AbstractMember
{
public:
  //################################################################################################
  //! Pass in the name used to uniquily identify this member.
  /*!
  \param name The name of this member unique within the collection that it is part of.
  \param type The typeof this member.
  */
  AbstractMember(const tp_utils::StringID& name, const tp_utils::StringID& type);

  //################################################################################################
  virtual ~AbstractMember();

  //################################################################################################
  //! The name of this member.
  /*!
  Each member has a name that is unique within the collection that it belongs.
  \returns The name of this member.
  */
  const tp_utils::StringID& name() const;

  //################################################################################################
  //! Set the name of this member.
  void setName(const tp_utils::StringID& name);

  //################################################################################################
  //! The type of this member.
  /*!
  The type of the member is used to select the correct factory for loading and saving it.
  \returns The type of this member.
  */
  const tp_utils::StringID& type() const;

  //################################################################################################
  //! When this data member was created.
  /*!
  This will be initialized to the current time but factories may change this.
  \return The timestamp of this member.
  */
  int64_t timestampMS()const;

  //################################################################################################
  //! set the timestamp of this member.
  void setTimestampMS(int64_t timestampMS);

  //################################################################################################
  virtual std::string toData() const;

private:
  tp_utils::StringID m_name;
  const tp_utils::StringID m_type;
  int64_t m_timestampMS;
};

//##################################################################################################
template <typename M, typename T>
std::shared_ptr<tp_data::AbstractMember> makeMember(const tp_utils::StringID& name, const T& data)
{
  auto member = new M(name);
  member->data = data;
  return std::shared_ptr<tp_data::AbstractMember>(member);
}

}

