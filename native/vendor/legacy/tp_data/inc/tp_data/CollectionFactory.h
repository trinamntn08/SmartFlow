#pragma once

#include "tp_data/Globals.h"

#include <memory>

namespace tp_data
{
class AbstractMember;
class AbstractMemberFactory;
class Collection;

//##################################################################################################
//! Used to load / save Collection objects.
/*!
This is used to load and save Collection objects, it works by delegating work to
AbstractMemberFactory objects. The addMemberFactory can be used to add factories for all of the
member types that the collection is expected to hold.

\note This class is thread safe provided that addMemberFactory is only called at construction.
*/
class CollectionFactory
{
  TP_DQ;
public:
  //################################################################################################
  CollectionFactory();

  //################################################################################################
  virtual ~CollectionFactory();

  //################################################################################################
  bool finalized() const;

  //################################################################################################
  void finalize();

  //################################################################################################
  //! Add a member factory
  /*!
  \note This will take ownership.
  \param memberFactory The new member factory to add.
  */
  void addMemberFactory(AbstractMemberFactory* memberFactory);

  //################################################################################################
  //! Returns the member factory for type or nullptr.
  const AbstractMemberFactory* memberFactory(const tp_utils::StringID& type) const;

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, std::unique_ptr<AbstractMemberFactory>>& memberFactories() const;

  //################################################################################################
  //! Clones a collection and adds it to the output.
  /*!
  This can be used to clone a collection or clone a collection and append its results to another
  collection.

  \note This will add to output, it will not clear any members already in there.

  \param error If something goes wrong this will be set to a description of the error.
  \param collection The Collection to clone.
  \param output A Collection that the data will be cloned into.
  \param subset If this is not empty only a subset of members will be cloned.
  */
  void cloneAppend(std::string& error,
                   const Collection& collection,
                   Collection& output,
                   const std::vector<std::string>& subset=std::vector<std::string>()) const;

  //################################################################################################
  //! Load a Collection from a blob of data.
  /*!
  \param error If something goes wrong this will be set to a description of the error.
  \param data The data to load from.
  \param output An empty Collection that the data will be loaded into.
  \param subset If this is not empty only a subset of members will be loaded.
  */
  void loadFromData(std::string& error,
                    const std::string& data,
                    Collection& output,
                    const std::vector<std::string>& subset=std::vector<std::string>()) const;

  //################################################################################################
  //! Load a Collection from a directory.
  /*!
  This loads the Collection from a directory containing a file for each member.

  \param error If something goes wrong this will be set to a description of the error.
  \param path A path to the directory to load from.
  \param output An empty Collection that the data will be loaded into.
  \param subset If this is not empty only a subset of members will be loaded.
  */
  void loadFromPath(std::string& error,
                    const std::string& path,
                    Collection& output,
                    const std::vector<std::string>& subset=std::vector<std::string>()) const;

  //################################################################################################
  //! Save a Collection to a blob of data.
  /*!
  \param error If something goes wrong this will be set to a description of the error.
  \param collection The Collection to save.
  \param data The output data.
  */
  void saveToData(std::string& error, const Collection& collection, std::string& data) const;

  //################################################################################################
  //! Save a Collection to a directory.
  /*!
  Save a Collection to a directory, each member will be saved in its own file. The output from this
  method is intended to be human readable.

  \param error If something goes wrong this will be set to a description of the error.
  \param collection The Collection to save.
  \param path The path to the output directory.
  \param append Append the collection to the existing contents of the path.
  */
  void saveToPath(std::string& error,
                  const Collection& collection,
                  const std::string& path,
                  bool append=false,
                  const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& nameMapping={}) const;

  //################################################################################################
  //! Return a clone of the member or nullptr on error.
  /*!
  \param error If something goes wrong this will be set to a description of the error.
  \param member The member to clone.
  \return The clone or nullptr.
  */
  std::shared_ptr<AbstractMember> clone(std::string& error, const AbstractMember& member) const;

  //################################################################################################
  //! Serialize a member into a binary data string.
  /*!
  This will find and use the appropriate AbstractMemberFactory to serialize a data member to a
  binary string. If a factory cant be found or any other error occours an error message will be
  written to the error parameter.

  \param error If something goes wrong this will be set to a description of the error.
  \param member The member to serialize.
  \param data The output data.
  \param extension The file extension to use if writing out to file.
  */
  void serializeMember(std::string& error,
                       const AbstractMember& member,
                       std::string& data,
                       std::string& extension) const;
};

}
