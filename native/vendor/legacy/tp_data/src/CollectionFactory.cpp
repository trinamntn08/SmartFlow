#include "tp_data/CollectionFactory.h"
#include "tp_data/AbstractMember.h"
#include "tp_data/AbstractMemberFactory.h"
#include "tp_data/Collection.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/JSONUtils.h"
#include "tp_utils/StackTrace.h"

#include "tp_utils/JSONUtils.h"

#include <memory>
#include <unordered_map>

namespace tp_data
{

namespace
{

//##################################################################################################
void addPart(std::string& output, const std::string& key, const std::string& data)
{
  auto keyLen = uint8_t(key.size());
  auto partLen = keyLen + data.size() + 5;

  output.reserve(output.size()+partLen);
  output.push_back(static_cast<char>(keyLen));
  output.append(key);

  output.push_back(static_cast<char>(data.size() >>  0));
  output.push_back(static_cast<char>(data.size() >>  8));
  output.push_back(static_cast<char>(data.size() >> 16));
  output.push_back(static_cast<char>(data.size() >> 24));

  output.append(data);
}

//##################################################################################################
bool parsePart(std::string& error, const std::string& input, int& startFrom, std::string& key, std::string& data)
{
  auto ok = [&input, &startFrom, &error](int count)
  {
    bool ok = ((startFrom+count)<=int(input.size()));
    if(!ok)
      error = "Unexpectedly reached end of buffer.";
    return ok;
  };

  //- Read a single unsigned byte that is the key length -------------------------------------------
  if(int(input.size())<=startFrom)
    return false;

  auto keyLen = static_cast<uint8_t>(input.at(size_t(startFrom)));
  startFrom++;


  //- Read the key ---------------------------------------------------------------------------------
  if(!ok(keyLen))
    return false;

  key.resize(keyLen);
  if(keyLen>0)
  {
    memcpy(&key[0], input.data()+startFrom, keyLen);
    startFrom += int(keyLen);
  }


  //- Read a 4 byte unsigned little endian number that is the length of the data -------------------
  if(!ok(4))
    return false;

  uint32_t dataLen = 0;
  for(uint32_t i=0; i<4; i++)
  {
    auto p = uint8_t(input.at(size_t(startFrom)));
    startFrom++;
    dataLen |= (uint32_t(p) << (8*i));
  }


  //- Read the data --------------------------------------------------------------------------------
  if(!ok(int(dataLen)))
    return false;

  data.resize(dataLen);
  if(dataLen>0)
  {
    memcpy(&data[0], input.data()+startFrom, dataLen);
    startFrom += int(dataLen);
  }

  return true;
}

}

//##################################################################################################
struct CollectionFactory::Private
{
  std::unordered_map<tp_utils::StringID, std::unique_ptr<AbstractMemberFactory>> memberFactories;
  bool finalized{false};
};

//##################################################################################################
CollectionFactory::CollectionFactory():
  d(new Private())
{

}

//##################################################################################################
CollectionFactory::~CollectionFactory()
{
  delete d;
}

//##################################################################################################
bool CollectionFactory::finalized() const
{
  return d->finalized;
}

//##################################################################################################
void CollectionFactory::finalize()
{
  if(d->finalized)
  {
    tpWarning() << "Error: CollectionFactory already finalized!";
    tp_utils::printStackTrace();
    return;
  }

  d->finalized = true;
}

//##################################################################################################
void CollectionFactory::addMemberFactory(AbstractMemberFactory* memberFactory)
{
  if(d->finalized)
  {
    tpWarning() << "Error: You can't add member factories to a CollectionFactory that has already been finalized!";
    tp_utils::printStackTrace();
    return;
  }

  d->memberFactories[memberFactory->type()].reset(memberFactory);
}


//##################################################################################################
const AbstractMemberFactory* CollectionFactory::memberFactory(const tp_utils::StringID& type) const
{
  auto i = d->memberFactories.find(type);
  return (i != d->memberFactories.end())?(i->second.get()):nullptr;
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, std::unique_ptr<AbstractMemberFactory>>& CollectionFactory::memberFactories() const
{
  return d->memberFactories;
}

//##################################################################################################
void CollectionFactory::cloneAppend(std::string& error,
                                    const Collection& collection,
                                    Collection& output,
                                    const std::vector<std::string>& subset) const
{
  for(const auto& member : collection.members())
  {
    if(!subset.empty() && !tpContains(subset, member->name()))
      continue;

    const tp_utils::StringID& type = member->type();

    auto factory=memberFactory(type);
    if(!factory)
    {
      error = "Failed to find factory for member type: " + type.toString();
      continue;
    }

    auto newMember = factory->clone(error, *member);
    if(!newMember)
    {
      error = "Failed to clone member of type: " + type.toString();
      continue;
    }

    newMember->setName(member->name());
    newMember->setTimestampMS(member->timestampMS());
    output.addMember(newMember);
  }
}

//##################################################################################################
void CollectionFactory::loadFromData(std::string& error,
                                     const std::string& data,
                                     Collection& output,
                                     const std::vector<std::string>& subset) const
{
  if(data.empty())
  {
    error = "Data is empty.";
    return;
  }

  bool headerSet=false;

  //These hold the details of the current member that we are parsing, once complete addMember()
  //will be called to add the member to the collection.
  std::string currentMemberName;
  int64_t currentMemberTimestamp{0};
  std::string currentMemberType;
  std::string currentMemberData;

  auto addMember = [&, this]()
  {
    if(currentMemberType.empty())
      return true;

    if(!subset.empty() && !tpContains(subset, currentMemberName))
      return true;

    headerSet = true;

    auto factory=memberFactory(currentMemberType);

    if(!factory)
    {
      tpWarning() << "Failed to find member factory for: " << currentMemberType;
      error = "Failed to find member factory for: " + currentMemberType;
      return false;
    }

    auto member = factory->load(error, currentMemberData);

    if(!member || !error.empty())
    {
      tpWarning() << "Valid: " << (member!=nullptr);
      tpWarning() << "Error: " << error;

      tpWarning() << "Failed to load a member, name: " << currentMemberName << " type: " << currentMemberType;
      error = "Failed to load a member, name: " + currentMemberName + " type: " + currentMemberType;
      return false;
    }

    member->setName(currentMemberName);
    member->setTimestampMS(currentMemberTimestamp);
    output.addMember(member);

    currentMemberType.clear();
    currentMemberName.clear();
    currentMemberTimestamp = 0;
    currentMemberData.clear();
    return true;
  };

  auto flushState = [&]()
  {
    return addMember();
  };

  int startFrom = 0;
  std::string key;
  std::string partData;
  while(parsePart(error, data, startFrom, key, partData))
  {
    if(key == "member")
    {
      if(!flushState())
      {
        error = "Flush state error.";
        return;
      }

      currentMemberName = partData;
    }

    else if(key == "type")
    {
      if(!currentMemberName.empty())
        currentMemberType = partData;
    }

    else if(key == "timestamp")
    {
      if(!currentMemberName.empty())
        currentMemberTimestamp = std::stoll(partData);
      else if(!headerSet)
        output.setTimestampMS(std::stoll(partData));
      else
        tpWarning() << "Unexpected timestamp.";
    }

    else if(key == "data")
    {
      if(!currentMemberName.empty())
        currentMemberData = partData;
    }

    else if(key == "name")
    {
      if(!headerSet)
        output.setName(partData);
    }
  }

  if(!flushState())
    error = "Final flush state error.";
}

//##################################################################################################
void CollectionFactory::loadFromPath(std::string& error,
                                     const std::string& path,
                                     Collection& output,
                                     const std::vector<std::string>& subset) const
{
  tp_utils::JSON j = tp_utils::readJSONFile(path + "/index.json");

  output.setName(TPJSONString(j, "name"));
  output.setTimestampMS(TPJSONInt64T(j, "timestamp"));

  if(const auto i=j.find("members"); i!=j.end() && i->is_array())
  {
    for(const auto& jj : *i)
    {
      auto name = TPJSONString(jj, "name");
      if(!subset.empty() && !tpContains(subset, name))
        continue;

      auto filename  = TPJSONString(jj, "filename");
      auto type      = TPJSONString(jj, "type");
      auto timestamp = TPJSONInt64T(jj, "timestamp");

      if(type.empty())
      {
        error = "Empty member type!";
        return;
      }

      auto factory = memberFactory(type);

      if(!factory)
      {
        tpWarning() << "Failed to find member factory for: " << type;
        error = "Failed to find member factory for: ";
        error += type;
        return;
      }

      std::string memberPath = path;
      memberPath += "/";
      memberPath += filename;
      auto member = factory->load(error, tp_utils::readBinaryFile(memberPath));


      if(!member || !error.empty())
      {
        tpWarning() << "Valid: " << (member!=nullptr);
        tpWarning() << "Error: " << error;

        tpWarning() << "Failed to load a member, name: " << name << " type: " << type;
        tpWarning() << "  -- From path: " << memberPath;
        error = "Failed to load a member, name: ";
        error += name;
        error += " type: ";
        error += type;
        return;
      }

      member->setName(name);
      member->setTimestampMS(timestamp);
      output.addMember(member);
    }
  }
}

//##################################################################################################
void CollectionFactory::saveToData(std::string& error, const Collection& collection, std::string& data) const
{
  addPart(data, "name", collection.name());
  addPart(data, "timestamp", std::to_string(collection.timestampMS()));

  for(const auto& member : collection.members())
  {
    const tp_utils::StringID& type = member->type();

    auto factory=memberFactory(type);

    if(!factory)
    {
      error = "Failed to find factory for member type: " + type.toString();
      return;
    }

    std::string timestamp = std::to_string(member->timestampMS());
    std::string memberData;
    factory->save(error, *member, memberData);

    if(!error.empty())
    {
      error += "Failed to serialize name:" + member->name().toString() + " type:" + type.toString();
      return;
    }

    addPart(data, "member", member->name().toString());
    addPart(data, "type", type.toString());
    addPart(data, "timestamp", timestamp);
    addPart(data, "data", memberData);
  }
}

//##################################################################################################
void CollectionFactory::saveToPath(std::string& error,
                                   const Collection& collection,
                                   const std::string& path,
                                   bool append,
                                   const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& nameMapping) const
{
#if 0
  tpWarning() << "CollectionFactory::saveToPath Not supported on this platform!";
  TP_UNUSED(error);
  TP_UNUSED(collection);
  TP_UNUSED(path);
#else
  tp_utils::JSON membersIndex = tp_utils::JSON::array();
  tp_utils::JSON existingMembersIndex = tp_utils::JSON::array();

  //-- Create the output directory -----------------------------------------------------------------
  if(tp_utils::exists(path))
  {
    if(!append)
    {
      error = "Output directory already exists.";
      return;
    }

    tp_utils::JSON j = tp_utils::readJSONFile(path + "/index.json");
    if(j.count("members") == 0)
    {
      error = "Failed to read existing index.";
      return;
    }

    existingMembersIndex = j["members"];
    if(!existingMembersIndex.is_array())
    {
      error = "Existing index is not an array.";
      return;
    }
  }

  else if(!tp_utils::mkdir(path, TPCreateFullPath::No))
  {
    error = "Failed to create output directory.";
    return;
  }

  //-- Save each member to its own file ------------------------------------------------------------
  std::vector<tp_utils::StringID> newMembers;
  newMembers.reserve(collection.members().size());
  for(const auto& member : collection.members())
  {
    auto mapName = [&](const tp_utils::StringID& name)
    {
      if(const auto i=nameMapping.find(name); i!=nameMapping.end())
        return i->second;
      return name;
    };

    const tp_utils::StringID& name = mapName(member->name());
    const tp_utils::StringID& type = member->type();

    auto factory=memberFactory(type);
    if(!factory)
    {
      error = "Failed to find factory for member type: " + type.toString();
      return;
    }

    std::string data;
    factory->save(error, *member, data);

    if(!error.empty())
    {
      error += "Failed to serialize name:" + name.toString() + " type:" + type.toString();
      return;
    }

    std::string filename = name.toString();
    filename += ".";
    filename += factory->extension();

    std::string filePath = path;
    filePath += "/";
    filePath += filename;

    tp_utils::writeBinaryFile(filePath, data);

    {
      newMembers.push_back(name);
      tp_utils::JSON j;
      j["name"] = name.toString();
      j["filename"] = filename;
      j["type"] = type.toString();
      membersIndex.push_back(j);
    }
  }

  //-- Add in existing members ---------------------------------------------------------------------
  for(const tp_utils::JSON& i : existingMembersIndex)
    if(auto name = TPJSONString(i, "name"); !name.empty() && !tpContains(newMembers, name))
      membersIndex.push_back(i);

  //-- Create the index file -----------------------------------------------------------------------
  {
    tp_utils::JSON j;

    j["name"] = collection.name();
    j["timestamp"] = collection.timestampMS();
    j["members"] = membersIndex;
    tp_utils::writePrettyJSONFile(path + "/index.json.new", j);
    //Move would be better.
    tp_utils::copyFile(path + "/index.json.new", path + "/index.json");
  }
#endif
}

//##################################################################################################
std::shared_ptr<AbstractMember> CollectionFactory::clone(std::string& error, const AbstractMember& member) const
{
  auto factory = memberFactory(member.type());
  if(!factory)
  {
    error = "Failed to find factory for member type: " + member.type().toString();
    return nullptr;
  }

  return factory->clone(error, member);
}

//##################################################################################################
void CollectionFactory::serializeMember(std::string& error,
                                        const AbstractMember& member,
                                        std::string& data,
                                        std::string& extension) const
{
  auto factory = memberFactory(member.type());
  if(!factory)
  {
    error = "Failed to find factory for member type: " + member.type().toString();
    return;
  }

  factory->save(error, member, data);
  extension = factory->extension();
}

}

