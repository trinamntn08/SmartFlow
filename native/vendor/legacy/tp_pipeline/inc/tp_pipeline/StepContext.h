#pragma once

#include "tp_pipeline/Globals.h"
#include "tp_pipeline/StepOutput.h"

#include "tp_data/Collection.h"

#include <unordered_set>

namespace tp_utils
{
class Progress;
}

namespace tp_data
{
class Collection;
class CollectionFactory;
}

namespace tp_pipeline
{
class PipelineManager;
class StepDelegate;

//##################################################################################################
struct SpecialDebugOutput
{
  std::string folderName;
  std::string filePath;
};

//##################################################################################################
struct TP_PIPELINE_SHARED_EXPORT StepContext
{
  const StepDelegate* stepDelegate{nullptr};
  StepDetails* stepDetails{nullptr};

  std::string debugRootDir;
  std::string stepDebugDir;
  std::vector<SpecialDebugOutput> specialDebugOutput;

  std::unordered_set<StepContext*> dependsOn;
  std::vector<std::pair<StepContext*, tp_utils::StringID>> inputDataSources;

  std::shared_ptr<tp_data::Collection> stepInput{};
  std::shared_ptr<StepOutput> stepOutput;
  tp_utils::Progress* progress{nullptr};
  bool runOk{false};

  bool runStarted{false};
  bool runComplete{false};
  bool runSkipped{false};
  bool executeWithCache{false}; //!< If true, try to use cached result if inputs match

  uint64_t readyToRunAt{0};
  uint64_t addedToTaskQueueAt{0};
  uint64_t runStartedAt{0};
  uint64_t runCompleteAt{0};

  uint64_t cacheKey{0};

  //################################################################################################
  bool dependenciesMet() const;

  //################################################################################################
  const tp_utils::StringID& inputDataNameFromPortName(const tp_utils::StringID& portName) const;

  //################################################################################################
  std::shared_ptr<tp_data::AbstractMember> member(const tp_utils::StringID& portName) const;

  //################################################################################################
  template<typename T>
  void memberCast(const tp_utils::StringID& portName, T*& member_) const
  {
    auto name = inputDataNameFromPortName(portName);
    if(!name.isValid())
      return;
    member_ = dynamic_cast<T*>(stepInput->member(name).get());
  }

  //################################################################################################
  template<typename T>
  T* memberCast(const tp_utils::StringID& portName) const
  {
    auto name = inputDataNameFromPortName(portName);
    if(!name.isValid())
      return nullptr;
    return dynamic_cast<T*>(stepInput->member(name).get());
  }

  //################################################################################################
  bool copyIntoCollection(const tp_data::CollectionFactory* collectionFactory,
                          tp_data::Collection& collection,
                          tp_utils::Progress* progress) const;
};

}
