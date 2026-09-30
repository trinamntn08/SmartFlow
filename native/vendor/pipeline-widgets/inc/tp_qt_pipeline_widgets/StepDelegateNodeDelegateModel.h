#pragma once

#include "tp_qt_pipeline_widgets/Globals.h"

#include "QtNodes/NodeDelegateModel"

namespace tp_pipeline
{
class StepDelegate;
}

namespace tp_qt_pipeline_widgets
{

//##################################################################################################
class TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT StepDelegateNodeDelegateModel : public QtNodes::NodeDelegateModel
{
  Q_OBJECT
  TP_DQ;
public:

  //################################################################################################
  StepDelegateNodeDelegateModel(const tp_pipeline::StepDelegate* stepDelegate);

  //################################################################################################
  ~StepDelegateNodeDelegateModel();

  //################################################################################################
  const tp_pipeline::StepDelegate* stepDelegate() const;

  //################################################################################################
  tp_pipeline::StepDetails* stepDetails() const;

  //################################################################################################
  void setStepDetails(tp_pipeline::StepDetails* stepDetails);

  //################################################################################################
  unsigned int nPorts(QtNodes::PortType portType) const override;

  //################################################################################################
  QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

  //################################################################################################
  void setConnectionAnchors(QtNodes::PortType portType,
                            QtNodes::PortIndex portIndex,
                            std::vector<QPointF> const& anchors) override;

  //################################################################################################
  std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex portIndex) override;

  //################################################################################################
  void setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex portIndex) override;

  //################################################################################################
  QString caption() const override;

  //################################################################################################
  QString name() const override;

  //################################################################################################
  QWidget* embeddedWidget() override;
};

}
