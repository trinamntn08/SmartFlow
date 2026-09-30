#pragma once

#include "tp_math_utils/Material.h"

#include "tp_utils/JSONUtils.h"

#include <vector>

namespace tp_math_utils
{

//##################################################################################################
class ExternalMaterialPayload
{
public:

  //################################################################################################
  virtual ~ExternalMaterialPayload();

  //################################################################################################
  virtual void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  virtual void loadState(const tp_utils::JSON& j);
};

//##################################################################################################
class TP_MATH_UTILS_EXPORT ExternalMaterial : public ExtendedMaterial
{
  mutable std::unique_ptr<ExternalMaterialPayload> externalMaterialPayload;
  tp_utils::JSON jPayload;
public:
  tp_utils::StringID assetType;
  tp_utils::StringID subPath;

  //################################################################################################
  ExternalMaterial();

  //################################################################################################
  ExternalMaterial(const ExternalMaterial& other);

  //################################################################################################
  ExternalMaterial(ExternalMaterial&& other) noexcept;

  //################################################################################################
  ExternalMaterial& operator=(const ExternalMaterial& other);

  //################################################################################################
  ExternalMaterial& operator=(ExternalMaterial&& other) noexcept;

  //################################################################################################
  template<typename T>
  T& getPayload()
  {
    if(auto c = dynamic_cast<T*>(externalMaterialPayload.get()); c)
      return *c;

    auto c = new T();
    c->loadState(jPayload);
    externalMaterialPayload.reset(c);
    return *c;
  }

  //################################################################################################
  template<typename T>
  const T& getPayload() const
  {
    if(auto c = dynamic_cast<T*>(externalMaterialPayload.get()); c)
      return *c;

    auto c = new T();
    c->loadState(jPayload);
    externalMaterialPayload.reset(c);
    return *c;
  }

  //################################################################################################
  void saveState(tp_utils::JSON& j) const override;

  //################################################################################################
  void loadState(const tp_utils::JSON& j) override;

  //################################################################################################
  void appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const override;

  //################################################################################################
  void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const override;

  //################################################################################################
  static void view(const Material& material,
                   const tp_utils::StringID& assetType,
                   const std::function<void(const ExternalMaterial&)>& closure);
};

}

