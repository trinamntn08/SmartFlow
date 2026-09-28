#include "tp_pipeline/ComplexObjectManager.h"
#include "tp_pipeline/ComplexObject.h"
#include "tp_pipeline/ComplexObjectFactory.h"

#include "tp_utils/JSONUtils.h"

namespace tp_pipeline
{

//##################################################################################################
struct ComplexObjectManager::Private
{
  std::unordered_map<tp_utils::StringID, ComplexObject*> complexObjects;
  std::unordered_map<tp_utils::StringID, tp_utils::JSON> complexObjectsJSON;
  std::unordered_map<tp_utils::StringID, std::vector<std::string>> complexObjectsBlobs;
};

//##################################################################################################
ComplexObjectManager::ComplexObjectManager():
  d(new Private)
{

}

//##################################################################################################
ComplexObjectManager::ComplexObjectManager(const ComplexObjectManager& other):
  d(new Private)
{
  d->complexObjectsJSON = other.d->complexObjectsJSON;
  d->complexObjectsBlobs = other.d->complexObjectsBlobs;

  for(const auto& i : other.d->complexObjects)
    d->complexObjects[i.first] = i.second->clone();
}

//##################################################################################################
ComplexObjectManager& ComplexObjectManager::operator=(const ComplexObjectManager& other)
{
  if(&other == this)
    return *this;

  d->complexObjectsJSON = other.d->complexObjectsJSON;
  d->complexObjectsBlobs = other.d->complexObjectsBlobs;

  for(const auto& i : other.d->complexObjects)
    d->complexObjects[i.first] = i.second->clone();

  return *this;
}

//##################################################################################################
ComplexObjectManager::~ComplexObjectManager()
{
  clearComplexObjects();
  delete d;
}

//##################################################################################################
void ComplexObjectManager::saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const noexcept
{
  auto& objects   = j["objects"];
  auto& blobIndex = j["blobIndex"];

  objects   = tp_utils::JSON::array();
  blobIndex = tp_utils::JSON::array();

  std::string currentObjectName;
  std::unordered_map<std::string, uint64_t> countsPerObject;
  auto addIndexedBlob = [&](const std::string& data)
  {
    auto index = countsPerObject[currentObjectName]++;
    tp_utils::JSON jj;
    jj["index"] = addBlob(data);
    jj["name"]  = currentObjectName;
    blobIndex.push_back(jj);
    return index;
  };

  for(const auto& i : d->complexObjects)
  {
    currentObjectName = i.first.toString();    
    tp_utils::JSON jj;
    jj["name"] = currentObjectName;
    i.second->saveBinary(jj["data"], addIndexedBlob);
    objects.push_back(jj);
  }

  for(const auto& i : d->complexObjectsJSON)
    objects.push_back({{"name", i.first.toString()}, {"data", i.second}});

  for(const auto& i : d->complexObjectsBlobs)
  {
    currentObjectName = i.first.toString();
    for(const auto& blob : i.second)
      addIndexedBlob(blob);
  }
}

//##################################################################################################
void ComplexObjectManager::loadBinary(const tp_utils::JSON& j, const std::vector<std::string>& blobs) noexcept
{
  clearComplexObjects();

  if(auto blobIndex = j.find("blobIndex"); blobIndex!=j.end() && blobIndex->is_array())
    for(const auto& jj : *blobIndex)
      if(auto index = TPJSONSizeT(jj, "index"); index<blobs.size())
        d->complexObjectsBlobs[TPJSONString(jj, "name")].push_back(blobs.at(index));

  if(auto objects = j.find("objects"); objects!=j.end() && objects->is_array())
  {
    for(tp_utils::JSON obj : *objects)
    {
      tp_utils::StringID name = TPJSONString(obj, "name", "");
      if(name.isValid())
        d->complexObjectsJSON[name] = obj["data"];
    }
  }
}

//##################################################################################################
void ComplexObjectManager::clearComplexObjects()
{
  for(const auto& i : d->complexObjects)
    delete i.second;

  d->complexObjectsJSON.clear();  
  d->complexObjectsBlobs.clear();
  d->complexObjects.clear();
}

//##################################################################################################
void ComplexObjectManager::removeComplexObject(const tp_utils::StringID& name)
{
  d->complexObjectsJSON.erase(name);
  d->complexObjectsBlobs.erase(name);

  auto i = d->complexObjects.find(name);
  if(i!=d->complexObjects.end())
  {
    delete i->second;
    d->complexObjects.erase(i);
  }
}

//##################################################################################################
ComplexObject* ComplexObjectManager::complexObject(const tp_utils::StringID& name, ComplexObjectFactory* factory)
{
  {
    auto i = d->complexObjects.find(name);
    if(i!=d->complexObjects.end())
      return i->second;
  }

  ComplexObject* obj = nullptr;
  {
    auto i = d->complexObjectsJSON.find(name);
    if(i!=d->complexObjectsJSON.end())
    {
      obj = factory->loadBinary(i->second, d->complexObjectsBlobs[name]);
      d->complexObjectsJSON.erase(i);

      if(auto i = d->complexObjectsBlobs.find(name); i!=d->complexObjectsBlobs.end())
        d->complexObjectsBlobs.erase(i);
    }
  }

  if(!obj)
    obj = factory->create();

  if(obj)
    d->complexObjects[name] = obj;

  return obj;
}

//##################################################################################################
bool ComplexObjectManager::isEmpty() const
{
  return d->complexObjects.empty() && d->complexObjectsJSON.empty() && d->complexObjectsBlobs.empty();
}

}
