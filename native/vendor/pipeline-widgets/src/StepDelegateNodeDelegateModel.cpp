#include "tp_qt_pipeline_widgets/StepDelegateNodeDelegateModel.h"

#include "tp_pipeline/StepDelegate.h"
#include "tp_pipeline/StepDetails.h"

namespace tp_qt_pipeline_widgets
{

namespace
{

//##################################################################################################
struct NodeData_lt : public QtNodes::NodeData
{
  QtNodes::NodeDataType dataType;

  //##################################################################################################
  QtNodes::NodeDataType type() const
  {
    return dataType;
  };
};

//##################################################################################################
struct InNodeDetails_lt
{
  QtNodes::NodeDataType dataType;
  std::weak_ptr<QtNodes::NodeData> data;
};
//##################################################################################################
struct OutNodeDetails_lt
{
  QtNodes::NodeDataType dataType;
  std::shared_ptr<QtNodes::NodeData> data;
};

}

//##################################################################################################
struct StepDelegateNodeDelegateModel::Private
{
  QString name;
  std::vector< InNodeDetails_lt> inPorts;
  std::vector<OutNodeDetails_lt> outPorts;

  const tp_pipeline::StepDelegate* stepDelegate;
  tp_pipeline::StepDetails* stepDetails{nullptr};

  //################################################################################################
  Private(const tp_pipeline::StepDelegate* stepDelegate_):
    stepDelegate(stepDelegate_)
  {

  }
};

//##################################################################################################
StepDelegateNodeDelegateModel::StepDelegateNodeDelegateModel(const tp_pipeline::StepDelegate* stepDelegate):
  d(new Private(stepDelegate))
{
  d->name = QString::fromStdString(stepDelegate->name().toString());

  // In Ports
  d->inPorts.reserve(stepDelegate->inPorts().size());
  for(const auto& port : stepDelegate->inPorts())
  {
    auto& portDetails = d->inPorts.emplace_back();

    portDetails.dataType.id = QString::fromStdString(port.type.toString());
    portDetails.dataType.name = QString::fromStdString(port.name.toString());
  }

  // Out Ports
  d->outPorts.reserve(stepDelegate->outPorts().size());
  for(const auto& port : stepDelegate->outPorts())
  {
    auto& portDetails = d->outPorts.emplace_back();

    portDetails.dataType.id = QString::fromStdString(port.type.toString());
    portDetails.dataType.name = QString::fromStdString(port.name.toString());

    NodeData_lt* nodeData = new NodeData_lt();
    nodeData->dataType = portDetails.dataType;
    portDetails.data.reset(nodeData);
  }
}

//##################################################################################################
StepDelegateNodeDelegateModel::~StepDelegateNodeDelegateModel()
{
  delete d;
}

//##################################################################################################
const tp_pipeline::StepDelegate* StepDelegateNodeDelegateModel::stepDelegate() const
{
  return d->stepDelegate;
}

//##################################################################################################
tp_pipeline::StepDetails* StepDelegateNodeDelegateModel::stepDetails() const
{
  return d->stepDetails;
}

//##################################################################################################
void StepDelegateNodeDelegateModel::setStepDetails(tp_pipeline::StepDetails* stepDetails)
{
  d->stepDetails = stepDetails;
}

//##################################################################################################
unsigned int StepDelegateNodeDelegateModel::nPorts(QtNodes::PortType portType) const
{
  switch(portType)
  {
    case QtNodes::PortType::In   : return int(d->inPorts .size());
    case QtNodes::PortType::Out  : return int(d->outPorts.size());
    case QtNodes::PortType::None : break;
  }

  return 0;
}

//##################################################################################################
QtNodes::NodeDataType StepDelegateNodeDelegateModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
  switch(portType)
  {
    case QtNodes::PortType::In   : return d->inPorts .at(size_t(portIndex)).dataType;
    case QtNodes::PortType::Out  : return d->outPorts.at(size_t(portIndex)).dataType;
    case QtNodes::PortType::None : break;
  }

  return {};
}

//##################################################################################################
void StepDelegateNodeDelegateModel::setConnectionAnchors(QtNodes::PortType portType,
                                                         QtNodes::PortIndex portIndex,
                                                         std::vector<QPointF> const& anchors)
{
  if(portType != QtNodes::PortType::In)
    return;

  if(!d->stepDetails)
    return;

  auto mapping = d->stepDetails->inputMapping();

  if(size_t(portIndex) < mapping.size())
  {
    std::vector<std::pair<float, float>> newAnchors;
    newAnchors.reserve(anchors.size());

    for(const auto& point : anchors)
      newAnchors.emplace_back(point.x(), point.y());

    mapping[size_t(portIndex)].anchors = newAnchors;

    d->stepDetails->setInputMapping(mapping);
  }
}

//##################################################################################################
std::shared_ptr<QtNodes::NodeData> StepDelegateNodeDelegateModel::outData(QtNodes::PortIndex portIndex)
{
  return d->outPorts.at(size_t(portIndex)).data;
}

//##################################################################################################
void StepDelegateNodeDelegateModel::setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex portIndex)
{
  d->inPorts.at(size_t(portIndex)).data = data;
}

//##################################################################################################
QString StepDelegateNodeDelegateModel::caption() const
{
  return d->name;
}

//##################################################################################################
QString StepDelegateNodeDelegateModel::name() const
{
  return d->name;
}

//##################################################################################################
QWidget* StepDelegateNodeDelegateModel::embeddedWidget()
{
  return nullptr;
}

}
