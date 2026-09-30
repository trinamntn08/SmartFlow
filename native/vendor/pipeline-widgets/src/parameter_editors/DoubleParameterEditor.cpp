#include "tp_qt_pipeline_widgets/parameter_editors/DoubleParameterEditor.h"

#include <QBoxLayout>
#include <QDoubleSpinBox>

namespace tp_qt_pipeline_widgets
{

//##################################################################################################
struct DoubleParameterEditor::Private
{
  QDoubleSpinBox* doubleSpinBox{nullptr};
};

//##################################################################################################
DoubleParameterEditor::DoubleParameterEditor(const tp_pipeline::Parameter& parameter):
  d(new Private())
{
  QVBoxLayout* layout = new QVBoxLayout(this);

  d->doubleSpinBox = new QDoubleSpinBox();
  layout->addWidget(d->doubleSpinBox);
  d->doubleSpinBox->setDecimals(10);
  d->doubleSpinBox->setRange(tpGetVariantValue<double>(parameter.min,   0.0),
                             tpGetVariantValue<double>(parameter.max, 100.0));
  d->doubleSpinBox->setValue(tpGetVariantValue<double>(parameter.value, 0.0));
}

//##################################################################################################
DoubleParameterEditor::~DoubleParameterEditor()
{
  delete d;
}

//##################################################################################################
void DoubleParameterEditor::updateParameter(tp_pipeline::Parameter& parameter)const
{
  parameter.value = d->doubleSpinBox->value();
}

}
