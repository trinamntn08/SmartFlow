#pragma once

#include "tp_data/AbstractMember.h"

#include "tp_utils/TPPixel.h"

#include "tp_utils/JSONUtils.h" // IWYU pragma: keep

namespace tp_data
{

//##################################################################################################
//! Used to load / save AbstractMember objects.
/*!
This should be subclassed to save different AbstractMember types.
*/
class AbstractMemberFactory
{
public:
  //################################################################################################
  //! Pass in the type of AbstractMember that this factory works with.
  /*!
  \param type The type of member this factory works with.
  \param extension The file extension to save these members with.
  */
  AbstractMemberFactory(const tp_utils::StringID& type, const std::string& extension, TPPixel color);

  //################################################################################################
  virtual ~AbstractMemberFactory();

  //################################################################################################
  //! The type of member this factory works with.
  /*!
  The member type is used to select a factory with a matching type.
  \returns The type of member that this factory works with.
  */
  const tp_utils::StringID& type() const;

  //################################################################################################
  //! The file extension for this type of member.
  /*!
  When this type of member is written to file it should be given this extension.
  \return The file extension for this type of member.
  */
  const std::string& extension() const;

  //################################################################################################
  //! Color used for rendering node graphs.
  TPPixel color() const;

  //################################################################################################
  //! Make a clone of the member.
  /*!
  Subclasses should reimplement this to make a clone of members.

  \param error This will be set on error.
  \param member The member to clone.
  \return A clone of the member of nullptr.
  */
  virtual std::shared_ptr<AbstractMember> clone(std::string& error, const AbstractMember& member) const=0;

  //################################################################################################
  //! Save the member to data.
  /*!
  Subclasses should reimplement this to save members.

  \param error This will be set on error.
  \param member The member to clone.
  \param data The output data.
  */
  virtual void save(std::string& error, const AbstractMember& member, std::string& data) const=0;

  //################################################################################################
  virtual std::shared_ptr<AbstractMember> load(std::string& error, const std::string& data) const=0;

private:
  const tp_utils::StringID m_type;
  std::string m_extension;
  TPPixel m_color;
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class JSONMemberFactoryTemplate : public AbstractMemberFactory
{
public:
  //################################################################################################
  JSONMemberFactoryTemplate(TPPixel color):
    AbstractMemberFactory(type_(), "json", color)
  {

  }

  //################################################################################################
  std::shared_ptr<AbstractMember> clone(std::string& error, const AbstractMember& member) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return {};
    }

    return makeMember<T>("", m->data);
  }

  //################################################################################################
  void save(std::string& error, const AbstractMember& member, std::string& data) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return;
    }

    data = m->toJSON().dump();
  }

  //################################################################################################
  std::shared_ptr<AbstractMember> load(std::string& error, const std::string& data) const override
  {
    try
    {
      auto j = tp_utils::JSON::parse(data);
      return std::shared_ptr<tp_data::AbstractMember>(T::fromJSON(j));
    }
    catch(...)
    {
      error = "Failed to parse JSON for " + type().toString();
      return {};
    }
  }
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class MultiDataMemberFactoryTemplate : public AbstractMemberFactory
{
public:
  //################################################################################################
  MultiDataMemberFactoryTemplate(TPPixel color):
    AbstractMemberFactory(type_(), T::extension, color)
  {

  }

  //################################################################################################
  std::shared_ptr<AbstractMember> clone(std::string& error, const AbstractMember& member) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return nullptr;
    }

    auto newMember = new T();
    newMember->data = m->data;
    return std::shared_ptr<AbstractMember>(newMember);
  }

  //################################################################################################
  void save(std::string& error, const AbstractMember& member, std::string& data) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return;
    }

    data = m->toData();
  }

  //################################################################################################
  std::shared_ptr<AbstractMember> load(std::string& error, const std::string& data) const override
  {
    return std::shared_ptr<AbstractMember>(T::fromData(error, data));
  }
};

//##################################################################################################
template<typename T, const tp_utils::StringID&(*type_)()>
class CloneableMemberFactoryTemplate : public AbstractMemberFactory
{
public:
  //################################################################################################
  CloneableMemberFactoryTemplate(TPPixel color):
    AbstractMemberFactory(type_(), T::extension, color)
  {

  }

  //################################################################################################
  std::shared_ptr<AbstractMember> clone(std::string& error, const AbstractMember& member) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return nullptr;
    }

    auto newMember = new T();
    newMember->clone(*m);
    return std::shared_ptr<AbstractMember>(newMember);
  }

  //################################################################################################
  void save(std::string& error, const AbstractMember& member, std::string& data) const override
  {
    auto m = dynamic_cast<const T*>(&member);
    if(!m)
    {
      error = "Failed to find member of type " + type().toString();
      return;
    }

    data = m->toData();
  }

  //################################################################################################
  std::shared_ptr<AbstractMember> load(std::string& error, const std::string& data) const override
  {
    return std::shared_ptr<AbstractMember>(T::fromData(error, data));
  }
};

}
