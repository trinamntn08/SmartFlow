#include "tp_pipeline/Globals.h"
#include "tp_pipeline/StepDelegateMap.h"
#include "tp_pipeline/Parameter.h"
#include "tp_pipeline/StepDetails.h"

#include "tp_pipeline/step_delegates/NoneStepDelegate.h"

#include "tp_data/CollectionFactory.h"

#include "tp_utils/DebugUtils.h"

//##################################################################################################
namespace tp_pipeline
{
TP_DEFINE_ID(                         noneSID,                             "None");
TP_DEFINE_ID(                         copySID,                             "Copy");
TP_DEFINE_ID(                          intSID,                              "Int");
TP_DEFINE_ID(                         sizeSID,                             "Size");
TP_DEFINE_ID(                        floatSID,                            "Float");
TP_DEFINE_ID(                       doubleSID,                           "Double");
TP_DEFINE_ID(                         enumSID,                             "Enum");
TP_DEFINE_ID(                    namedDataSID,                       "Named data");
TP_DEFINE_ID(                    directorySID,                        "Directory");
TP_DEFINE_ID(                       stringSID,                           "String");
TP_DEFINE_ID(                   binaryDataSID,                      "Binary data");
TP_DEFINE_ID(            convolutionMatrixSID,               "Convolution matrix");
TP_DEFINE_ID(                fileDirectorySID,                   "File directory");
TP_DEFINE_ID(                    fileIndexSID,                       "File index");
TP_DEFINE_ID(                   collectionSID,                       "Collection");
TP_DEFINE_ID(                  memberNamesSID,                     "Member names");
TP_DEFINE_ID(                       inputsSID,                           "Inputs");
TP_DEFINE_ID(                      outputsSID,                          "Outputs");
TP_DEFINE_ID(                 pipelineNameSID,                    "Pipeline name");
TP_DEFINE_ID(                       sourceSID,                           "Source");
TP_DEFINE_ID(           fallbackOperationsSID,              "Fallback operations");
TP_DEFINE_ID(           specialDebugOutputSID,             "Special debug output");

//##################################################################################################
void createStepDelegates(StepDelegateMap& stepDelegates, const tp_data::CollectionFactory* collectionFactory)
{
  TP_UNUSED(collectionFactory);
  stepDelegates.addStepDelegate(new NoneStepDelegate);
}

//##################################################################################################
void createAllStepDelegates(StepDelegateMap& stepDelegates, const tp_data::CollectionFactory* collectionFactory)
{
  if(!collectionFactory->finalized())
    tpWarning() << "createAllStepDelegates Error: The collection factory should be finalized!";

  for(const auto& createStepDelegates : tp_pipeline::createStepDelegatesRegister())
    createStepDelegates(stepDelegates, collectionFactory);
}

//##################################################################################################
std::vector<std::function<void(StepDelegateMap&, const tp_data::CollectionFactory*) > > & createStepDelegatesRegister()
{
  static std::vector<std::function<void(StepDelegateMap&, const tp_data::CollectionFactory*)>> createStepDelegatesRegister;
  return createStepDelegatesRegister;
}

//##################################################################################################
std::string stepStatusToString(StepStatus stepStatus)
{
  switch(stepStatus)
  {
  case StepStatus::NotRun : return "NotRun" ;
  case StepStatus::Success: return "Success";
  case StepStatus::Warning: return "Warning";
  case StepStatus::Failed : return "Failed" ;
  case StepStatus::Skipped: return "Skipped";
  }

  return "NotRun";
}

//##################################################################################################
StepStatus stepStatusFromString(const std::string& stepStatus)
{
  if(stepStatus == "NotRun" ) return StepStatus::NotRun ;
  if(stepStatus == "Success") return StepStatus::Success;
  if(stepStatus == "Warning") return StepStatus::Warning;
  if(stepStatus == "Failed" ) return StepStatus::Failed ;
  if(stepStatus == "Skipped") return StepStatus::Skipped;

  return StepStatus::NotRun;
}

//##################################################################################################
void setInputDirectory(const std::vector<StepDetails*>& steps, const std::string& directory)
{
  const auto& name = fileDirectorySID();
  for(tp_pipeline::StepDetails* stepDetails : steps)
  {
    const std::unordered_map<tp_utils::StringID, Parameter>& parameters = stepDetails->parameters();
    if(tpContainsKey(parameters, name))
    {
      tp_pipeline::Parameter param = tpGetMapValue(parameters, name);
      param.name = name;
      param.value = directory;
      stepDetails->setParamerter(param);
      break;
    }
  }
}

//##################################################################################################
tp_utils::StringID randomId()
{
  static std::random_device randomDevice;
  static std::mt19937 generator{randomDevice()};
  static const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::uniform_int_distribution<> distribution(0, int(characters.size() - 1));

  std::string randomString;
  for (size_t i=0; i<15; i++)
    randomString += characters[distribution(generator)];

  return randomString;
}

REGISTER_CREATE_STEP_DELEGATES;

//##################################################################################################
int staticInit()
{
  return 0;
}

}
