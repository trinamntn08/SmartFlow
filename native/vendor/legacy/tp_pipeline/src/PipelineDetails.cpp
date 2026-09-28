#include "tp_pipeline/PipelineDetails.h"

#include "tp_utils/StackTrace.h"
#include "tp_utils/DebugUtils.h"

namespace tp_pipeline
{

//##################################################################################################
struct PipelineDetails::Private
{
  TP_REF_COUNT_OBJECTS("PipelineDetails");
  Q* q;

  std::vector<StepDetails*> steps;

  bool blockCallbacks{false};

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(!steps.empty())
      tp_utils::printStackTrace();
  }

  //################################################################################################
  void copy(const PipelineDetails& other)
  {
    for(StepDetails* step : other.d->steps)
    {
      StepDetails* newStep = new StepDetails(*step);
      newStep->setParent(q);
      steps.push_back(newStep);
    }
  }
};

//##################################################################################################
PipelineDetails::PipelineDetails():
  d(new Private(this))
{

}

//##################################################################################################
PipelineDetails::PipelineDetails(const PipelineDetails& other):
  d(new Private(this))
{  
  d->copy(other);
}

//##################################################################################################
PipelineDetails::~PipelineDetails()
{
  for(StepDetails* step : d->steps)
    delete step;
  d->steps.clear();
  delete d;
}

//##################################################################################################
PipelineDetails& PipelineDetails::operator=(const PipelineDetails& other)
{
  while(!d->steps.empty())
    deleteStep(d->steps.at(0));

  d->copy(other);

  for(size_t i=0; i<d->steps.size(); i++)
    invalidateStep(d->steps.at(i));

  return *this;
}

//##################################################################################################
void PipelineDetails::printStats() const
{
  tpWarning() << "-- PipelineDetails::printStats -------------------------------------------------";
  tpWarning() << "nSteps: " << d->steps.size();
  for(const auto& step  : d->steps)
    tpWarning() << " - Step: " << step->delegateName().toString();
  tpWarning() << "--------------------------------------------------------------------------------";

}

//##################################################################################################
void PipelineDetails::appendPipeline(const PipelineDetails& other)
{
  std::unordered_map<tp_utils::StringID, tp_utils::StringID> dataIdMap;
  std::vector<StepDetails*> newSteps;

  // Pass 1: Clone steps and generate new unique IDs for their outputs
  // This prevents collisions with existing data in the current pipeline.
  for(const auto* step : other.steps())
  {
    if(step)
    {
      auto* newStep = new StepDetails(*step);
      newStep->generateNewOutputIds(dataIdMap);
      newSteps.push_back(newStep);
    }
  }

  // Pass 2: Update inputs to point to the new output IDs
  // This preserves the connections internally to the imported group.
  d->blockCallbacks = true;
  TP_CLEANUP([&]
  {
    d->blockCallbacks = false;
    for(auto* step : newSteps)
      changedCallback(step);
  });

  for(auto* newStep : newSteps)
  {
    newStep->updateInputIds(dataIdMap);
    addStep(newStep);
  }
}

//##################################################################################################
const std::vector<StepDetails*>& PipelineDetails::steps()const
{
  return d->steps;
}

//##################################################################################################
StepDetails* PipelineDetails::findStepFromDelegateName(const tp_utils::StringID& delegateName) const
{
  for(auto stepDetails : d->steps)
    if(stepDetails->delegateName() == delegateName)
      return stepDetails;

  return nullptr;
};

//##################################################################################################
StepDetails* PipelineDetails::findStepFromStepId(const tp_utils::StringID& stepId) const
{
  for(auto stepDetails : d->steps)
    if(stepDetails->id() == stepId)
      return stepDetails;

  return nullptr;
};

//##################################################################################################
void PipelineDetails::addStep(StepDetails* step)
{
  insertStep(step, d->steps.size());
}

//##################################################################################################
void PipelineDetails::insertStep(StepDetails* step, size_t index)
{
  if(step->parent())
  {
    if(step->parent() == this)
    {
      if(auto i = std::find(d->steps.begin(), d->steps.end(), step); i!=d->steps.end())
        d->steps.erase(i);
    }
    else
    {
      tpWarning() << "PipelineDetails::addStep() Error! Step already has a parent.";
      return;
    }
  }
  else
    step->setParent(this);


  d->steps.insert(d->steps.begin()+long(index), step);
  invalidateStep(step);
}

//##################################################################################################
void PipelineDetails::invalidateStep(StepDetails* step)
{
  if(!d->blockCallbacks)
    changedCallback(step);
}

//##################################################################################################
void PipelineDetails::deleteStep(StepDetails* step)
{
  auto it = std::find(d->steps.begin(), d->steps.end(), step);
  if(it!=d->steps.end())
    d->steps.erase(it);

  deletedCallback(step);

  delete step;
}

//##################################################################################################
void PipelineDetails::clearDanglingInputs()
{
  std::unordered_set<tp_utils::StringID> validOutputs;
  for(auto& step : d->steps)
  {
    bool changed=false;
    auto mapping = step->outputMapping();

    for(auto& outPort : mapping)
    {
      if(tpContains(validOutputs, outPort.dataName))
      {
        outPort.dataName = randomId();
        changed = true;
      }

      validOutputs.insert(outPort.dataName);
    }

    if(changed)
      step->setOutputMapping(mapping);
  }

  for(auto& step : d->steps)
  {
    bool changed=false;
    auto mapping = step->inputMapping();

    for(auto& outPort : mapping)
    {
      if(!tpContains(validOutputs, outPort.dataName))
      {
        outPort.dataName = {};
        changed = true;
      }
    }

    if(changed)
      step->setInputMapping(mapping);
  }
}

//##################################################################################################
void PipelineDetails::saveBinary(std::string& data)
{
  std::vector<std::string> blobs;
  auto addBlob = [&](const std::string& blob)
  {
    auto i=blobs.size();
    blobs.push_back(blob);
    return i;
  };

  tp_utils::JSON j;
  {
    auto& jj=j["Steps"];
    jj = tp_utils::JSON::array();
    jj.get_ptr<tp_utils::JSON::array_t*>()->reserve(d->steps.size());
    for(StepDetails* step : d->steps)
    {
      jj.emplace_back();
      step->saveBinary(jj.back(), addBlob);
    }
  }

  addBlob(j.dump());

  uint64_t indexSize = (blobs.size()*sizeof(uint64_t)*2)+sizeof(uint64_t);
  std::string index;
  index.reserve(indexSize);
  auto addNumber = [&](uint64_t number)
  {
    index.push_back(static_cast<char>(uint8_t(number>> 0)));
    index.push_back(static_cast<char>(uint8_t(number>> 8)));
    index.push_back(static_cast<char>(uint8_t(number>>16)));
    index.push_back(static_cast<char>(uint8_t(number>>24)));

    index.push_back(static_cast<char>(uint8_t(number>>32)));
    index.push_back(static_cast<char>(uint8_t(number>>40)));
    index.push_back(static_cast<char>(uint8_t(number>>48)));
    index.push_back(static_cast<char>(uint8_t(number>>56)));
  };

  addNumber(blobs.size());

  uint64_t total = indexSize;
  for(const auto& blob : blobs)
  {
    addNumber(total);
    addNumber(blob.size());
    total += blob.size();
  }

  data.reserve(total);
  data.append(index);
  for(const auto& blob : blobs)
    data.append(blob);
}

//##################################################################################################
void PipelineDetails::loadBinary(std::string& error, const std::string& data)
{
  if(data.size()<sizeof(uint64_t))
  {
    error = "Data is smaller than the indexes headder.";
    return;
  }

  const char* indexPtr = data.data();
  auto readNumber = [&]()
  {
    uint64_t number=0;

    number |= uint64_t(uint8_t(*indexPtr))<< 0; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<< 8; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<<16; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<<24; indexPtr++;

    number |= uint64_t(uint8_t(*indexPtr))<<32; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<<40; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<<48; indexPtr++;
    number |= uint64_t(uint8_t(*indexPtr))<<56; indexPtr++;

    return number;
  };

  uint64_t numberOfBlobs = readNumber();
  if(numberOfBlobs<1)
  {
    error = "No blobs found in data.";
    return;
  }

  size_t indexSize = (numberOfBlobs*sizeof(uint64_t)*2)+sizeof(uint64_t);
  if(indexSize>data.size())
  {
    error = "Index size is larger than source data.";
    return;
  }

  std::vector<std::string> blobs;
  uint64_t total = indexSize;
  for(uint64_t b=0; b<numberOfBlobs; b++)
  {
    uint64_t blobOffset = readNumber();
    if(total != blobOffset)
    {
      error = "Blob offset missmatch.";
      return;
    }

    uint64_t blobSize = readNumber();
    total += blobSize;
    if(total > data.size())
    {
      error = "Blob overflow.";
      return;
    }

    blobs.push_back(data.substr(blobOffset, blobSize));
  }

  if(blobs.empty())
  {
    error = "Failed to read any blobs.";
    return;
  }

  {
    auto j = tp_utils::JSON::parse(blobs.back());

    d->blockCallbacks = true;
    TP_CLEANUP([&]
    {
      d->blockCallbacks = false;
      for(auto step : d->steps)
        changedCallback(step);
    });

    while(!d->steps.empty())
      deleteStep(d->steps.at(0));

    if(auto i=j.find("Steps"); i!=j.end() && i->is_array())
    {
      for(const auto& stepData : *i)
      {
        StepDetails* step = new StepDetails();
        step->loadBinary(stepData, blobs);
        addStep(step);
      }
    }
  }
}

}
