#pragma once

#include "tp_qt_pipeline_widgets/Globals.h"

#include <tp_pipeline/Parameter.h>

#include <QWidget>

namespace tp_qt_pipeline_widgets
{

//##################################################################################################
class TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT AbstractParameterEditor: public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  AbstractParameterEditor();

  //################################################################################################
  virtual ~AbstractParameterEditor();

  //################################################################################################
  //! Update the parameter with the state from the widget
  virtual void updateParameter(tp_pipeline::Parameter& parameter)const=0;
};

//##################################################################################################
class AbstractParameterEditorFactory
{
public:
  //################################################################################################
  virtual ~AbstractParameterEditorFactory();

  //################################################################################################
  virtual AbstractParameterEditor* create(const tp_pipeline::Parameter& parameter)const=0;
};

//##################################################################################################
template<typename T>
class TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT ParameterEditorFactory : public AbstractParameterEditorFactory
{
public:
  //################################################################################################
  AbstractParameterEditor* create(const tp_pipeline::Parameter& parameter)const
  {
    return new T(parameter);
  }
};

}
