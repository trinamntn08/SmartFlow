#pragma once

#include "tp_qt_pipeline_widgets/AbstractParameterEditor.h"

namespace tp_qt_pipeline_widgets
{

//##################################################################################################
class TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT DoubleParameterEditor: public AbstractParameterEditor
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  DoubleParameterEditor(const tp_pipeline::Parameter& parameter);

  //################################################################################################
  virtual ~DoubleParameterEditor();

  //################################################################################################
  virtual void updateParameter(tp_pipeline::Parameter& parameter)const;
};


//##################################################################################################
typedef ParameterEditorFactory<DoubleParameterEditor> DoubleParameterEditorFactory;

}
