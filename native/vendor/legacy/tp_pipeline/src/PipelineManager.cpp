#include "tp_pipeline/PipelineManager.h"
#include "tp_pipeline/StepDelegate.h"
#include "tp_pipeline/StepDelegateMap.h"
#include "tp_pipeline/PipelineDetails.h"

#include "tp_data/Collection.h"
#include "tp_data/CollectionFactory.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/Progress.h"
#include "tp_utils/Hash.h"

#include <unordered_map>
#include <unordered_set>

namespace tp_pipeline
{

//##################################################################################################
struct PipelineManager::Private
{
  Q* q;
  PipelineDetails* pipelineDetails;
  const tp_pipeline::StepDelegateMap* stepDelegates;
  const tp_data::CollectionFactory* collectionFactory;
  bool fixupParameters;

  std::string debugRootDir;

  std::vector<StepContext> stepContexts;

  std::vector<StepContext*> requiredSteps;

  bool ignoreCallbacks{false};

  //################################################################################################
  Private(Q* q_,
          PipelineDetails* pipelineDetails_,
          const tp_pipeline::StepDelegateMap* stepDelegates_,
          const tp_data::CollectionFactory* collectionFactory_,
          bool fixupParameters):
    q(q_),
    pipelineDetails(pipelineDetails_),
    stepDelegates(stepDelegates_),
    collectionFactory(collectionFactory_),
    fixupParameters(fixupParameters)
  {
    rebuildContexts();
  }

  //################################################################################################
  tp_utils::Callback<void(StepDetails*)> changedCallback = [&](StepDetails* stepDetails)
  {
    TP_UNUSED(stepDetails);
    if(ignoreCallbacks)
      return;

    rebuildContexts();
  };

  //################################################################################################
  tp_utils::Callback<void(StepDetails*)> deletedCallback = [&](StepDetails* stepDetails)
  {
    TP_UNUSED(stepDetails);
    if(ignoreCallbacks)
      return;

    rebuildContexts();
  };

  //################################################################################################
  size_t indexOf(StepDetails* stepDetails)
  {
    for(size_t i=0; i<stepContexts.size(); i++)
      if(stepContexts.at(i).stepDetails == stepDetails)
        return i;

    return 0;
  }

  //################################################################################################
  StepContext* stepContextFor(StepDetails* stepDetails)
  {
    for(auto& stepContext : stepContexts)
      if(stepContext.stepDetails == stepDetails)
        return &stepContext;
    return nullptr;
  }

  //################################################################################################
  void rebuildContexts()
  {
    ignoreCallbacks=true;
    TP_CLEANUP([&]{ignoreCallbacks=false;});

    std::string stepsDebugRootDir;
    if(!debugRootDir.empty())
    {
      stepsDebugRootDir = tp_utils::pathAppend(debugRootDir, "steps");
      tp_utils::mkdir(stepsDebugRootDir, TPCreateFullPath::No);
    }

    const auto& steps = pipelineDetails->steps();
    requiredSteps.clear();
    requiredSteps.reserve(steps.size());
    stepContexts.resize(steps.size());
    for(size_t i=0; i<stepContexts.size(); i++)
    {
      auto stepDetails = steps.at(i);
      auto& stepContext = stepContexts.at(i);
      auto stepDelegate = stepDelegates->stepDelegate(stepDetails->delegateName());

      stepContext = {};
      stepContext.stepDetails = stepDetails;
      stepContext.stepDelegate = stepDelegate;

      if(!stepsDebugRootDir.empty())
      {
        stepContext.debugRootDir = stepsDebugRootDir;
        stepContext.stepDebugDir = tp_utils::pathAppend(stepContext.debugRootDir, stepDetails->delegateName().toString() + stepDetails->id().toString());
        tp_utils::mkdir(stepContext.stepDebugDir, TPCreateFullPath::No);
      }

      stepContext.stepInput = std::make_shared<tp_data::Collection>();

      if(stepContext.stepDelegate)
      {
        stepContext.stepOutput = std::make_shared<StepOutput>(stepContext.stepDelegate, stepContext.stepDetails);

        if(fixupParameters)
        {
          std::vector<tp_utils::StringID> validParams;
          stepContext.stepDelegate->fixupParameters(stepDetails, validParams);
          stepDetails->setParametersOrder(validParams);
          stepDetails->setValidParameters(validParams);

          {
            bool changed=false;
            std::vector<PortMapping> outputMapping = stepDetails->outputMapping();

            const auto& outPorts = stepContext.stepDelegate->outPorts();
            outputMapping.resize(outPorts.size());
            for(size_t c=0; c<outPorts.size(); c++)
            {
              auto& m = outputMapping.at(c);
              const auto& p = outPorts.at(c);
              if(m.portName != p.name || m.portType != p.type)
              {
                changed = true;
                m.portName = p.name;
                m.portType = p.type;
                m.dataName = randomId();
              }
            }

            if(changed)
              stepDetails->setOutputMapping(outputMapping);
          }

          {
            bool changed=false;
            std::vector<PortMapping> inputMapping = stepDetails->inputMapping();

            const auto& inPorts = stepContext.stepDelegate->inPorts();
            inputMapping.resize(inPorts.size());
            for(size_t c=0; c<inPorts.size(); c++)
            {
              auto& m = inputMapping.at(c);
              const auto& p = inPorts.at(c);
              if(m.portName != p.name || m.portType != p.type)
              {
                changed = true;
                m.portName = p.name;
                m.portType = p.type;
                m.dataName = {};
              }
            }

            if(changed)
              stepDetails->setInputMapping(inputMapping);
          }
        }
      }
    }

    if(fixupParameters)
      pipelineDetails->clearDanglingInputs();
  }

  //################################################################################################
  void clearDependencyState()
  {
    for(auto& stepContext : stepContexts)
    {
      stepContext.dependsOn.clear();
      stepContext.inputDataSources.clear();
    }
  }

  //################################################################################################
  void resetAndBuildDependencies()
  {
    std::unordered_map<tp_utils::StringID, StepContext*> outputProviders;
    size_t totalOutputMappings{0};

    clearDependencyState();

    // Step 1: Build a lookup from each output data name to the step that produces it.
    for(auto& stepContext : stepContexts)
      totalOutputMappings += stepContext.stepDetails->outputMapping().size();

    outputProviders.reserve(totalOutputMappings);
    for(auto& stepContext : stepContexts)
    {
      for(const auto& mapping : stepContext.stepDetails->outputMapping())
      {
        if(mapping.dataName.isValid())
          outputProviders[mapping.dataName] = &stepContext;
      }
    }

    // Step 2: For each step, resolve its required inputs to upstream producer steps.
    for(auto& stepContext : stepContexts)
    {
      std::unordered_set<tp_utils::StringID> requiredData;
      requiredData.reserve(stepContext.stepDetails->inputMapping().size());
      for(const auto& m : stepContext.stepDetails->inputMapping())
        if(m.dataName.isValid())
          requiredData.insert(m.dataName);

      stepContext.dependsOn.reserve(requiredData.size());
      stepContext.inputDataSources.reserve(requiredData.size());

      for(const auto& dataName : requiredData)
      {
        auto it = outputProviders.find(dataName);
        if(it == outputProviders.end() || it->second == &stepContext)
          continue;

        stepContext.dependsOn.insert(it->second);
        stepContext.inputDataSources.emplace_back(it->second, dataName);
      }
    }
  }

  //################################################################################################
  void addStepAndDependencies(StepContext* stepContext)
  {
    if(tpContains(requiredSteps, stepContext))
      return;

    requiredSteps.push_back(stepContext);

    for(auto s : stepContext->dependsOn)
      addStepAndDependencies(s);
  }

  //################################################################################################
  void addAllSteps()
  {
    requiredSteps.clear();
    for(auto& s : stepContexts)
      requiredSteps.push_back(&s);
  }
};

//##################################################################################################
PipelineManager::PipelineManager(PipelineDetails* pipelineDetails,
                                 const tp_pipeline::StepDelegateMap* stepDelegates,
                                 const tp_data::CollectionFactory* collectionFactory,
                                 bool fixupParameters):
  d(new Private(this, pipelineDetails, stepDelegates, collectionFactory, fixupParameters))
{
  d->changedCallback.connect(d->pipelineDetails->changedCallback);
  d->deletedCallback.connect(d->pipelineDetails->deletedCallback);
}

//##################################################################################################
PipelineManager::~PipelineManager()
{
  delete d;
}

//##################################################################################################
PipelineDetails* PipelineManager::pipelineDetails() const
{
  return d->pipelineDetails;
}

//##################################################################################################
void PipelineManager::setDebugRootDir(const std::string& debugRootDir)
{
  d->debugRootDir = debugRootDir;
}

//##################################################################################################
const std::string& PipelineManager::debugRootDir() const
{
  return d->debugRootDir;
}

//##################################################################################################
void PipelineManager::startExecution(StepDetails* finalStep)
{
  d->rebuildContexts();
  d->resetAndBuildDependencies();

  if(finalStep)
  {
    if(auto s=d->stepContextFor(finalStep); s)
      d->addStepAndDependencies(s);
  }
  else
    d->addAllSteps();
}

//##################################################################################################
StepContext* PipelineManager::takeNextAvailableStep(tp_utils::ParrallelProgress* parrallelProgress)
{
  for(auto s : d->requiredSteps)
  {
    if(s->runStarted==false && s->dependenciesMet())
    {
      s->runStarted = true;

      std::string delegateName{"<unknown>"};
      if(s->stepDelegate)
        delegateName = s->stepDelegate->name().toString();

      s->progress = parrallelProgress->addChildStep("Execute step: " + delegateName);

      if(s->readyToRunAt == 0)
        s->readyToRunAt = tp_utils::currentTimeMicroseconds();
      
      if(!s->stepDelegate)
        return s;

      for(const auto& m : s->stepDetails->inputMapping())
      {
        if(!m.dataName.isValid())
          continue;

        for(const auto& f : s->inputDataSources)
        {
          if(f.second == m.dataName)
          {
            if(!f.first->stepOutput)
              s->progress->addError("The delegate for the previous step that provides: '" + m.dataName.toString() + "' was not found!");
            else
            {
              auto member = f.first->stepOutput->output()->member(m.dataName);
              if(member)
                s->stepInput->addMember(member);
              else
                s->progress->addError("Failed to copy member: " + m.dataName.toString());
            }

            break;
          }
        }
      }

      return s;
    }
  }

  return nullptr;
}

//##################################################################################################
void PipelineManager::returnCompletedStep(StepContext* stepContext)
{
  stepContext->runComplete = true;

  for(auto s : d->requiredSteps)
  {
    if(s->runStarted==false && s->dependenciesMet())
    {
      if(s->readyToRunAt == 0)
        s->readyToRunAt = tp_utils::currentTimeMicroseconds();
    }
  }
}

//##################################################################################################
StepContext* PipelineManager::stepContext(StepDetails* stepDetails) const
{
  return d->stepContextFor(stepDetails);
}

//##################################################################################################
const std::vector<StepContext>& PipelineManager::stepContexts() const
{
  return d->stepContexts;
}

//##################################################################################################
void PipelineManager::printRunStats(tp_utils::Progress* progress) const
{
  size_t nameLenght=1;
  for(const auto& stepContext : d->stepContexts)
    nameLenght = std::max(stepContext.stepDetails->delegateName().toString().size(), nameLenght);

  for(const auto& stepContext : d->stepContexts)
  {
    const auto& name = stepContext.stepDetails->delegateName();    

    auto formatT = [](uint64_t t)
    {
      return tp_utils::fixedWidthKeepRight(std::to_string(t), 10, ' ');
    };

    auto wait = formatT(stepContext.addedToTaskQueueAt - stepContext.      readyToRunAt);
    auto tque = formatT(stepContext.      runStartedAt - stepContext.addedToTaskQueueAt);
    auto exec = formatT(stepContext.     runCompleteAt - stepContext.      runStartedAt);

    progress->addMessage(tp_utils::fixedWidthKeepLeft(name.toString(), nameLenght, ' ') +
                         ", wait: "  + wait +
                         ", queue: " + tque +
                         ", exec: "  + exec);
  }
}

//##################################################################################################
uint64_t PipelineManager::calculateCacheKey(const StepContext* stepContext) const
{
  if(!stepContext || !stepContext->stepDetails)
    return 0;

  uint64_t cacheKey = 0;

  hash_combine(cacheKey, stepContext->stepDetails->delegateName());

  // Hash the inputs
  for(const auto& mapping : stepContext->stepDetails->inputMapping())
  {
    if(mapping.dataName.isValid())
    {
      auto member = stepContext->stepInput->member(mapping.dataName);
      if(member)
      {
        tp_utils::hash_combine(cacheKey, member->timestampMS());
      }
    }
  }

  // Hash the parameters
  for(const auto& [name, param] : stepContext->stepDetails->parameters())
  {
    tp_utils::hash_combine(cacheKey, name);
    tp_utils::hash_combine(cacheKey, variantToString(param.value));
  }

  return cacheKey;
}

}
