#pragma once

#include "tp_utils/StringID.h" // IWYU pragma: keep

#include <QColor>

#include <unordered_map>

#if defined(TP_QT_PIPELINE_WIDGETS_LIBRARY)
#  define TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT TP_EXPORT
#else
#  define TP_QT_PIPELINE_WIDGETS_SHARED_EXPORT TP_IMPORT
#endif

#define ROLE_ITEM_TYPE        Qt::UserRole+101
#define ROLE_PIPELINE_POINTER Qt::UserRole+102
#define ROLE_STEP_POINTER     Qt::UserRole+103

#define ITEM_TYPE_PIPELINE    101 //A pipeline folder in the step tree
#define ITEM_TYPE_STEP        102 //A step item in the step tree
#define ITEM_TYPE_ERROR       103 //A step error item in the step tree

namespace tp_pipeline
{
class StepDetails;
}

namespace tp_data
{
class CollectionFactory;
}

//##################################################################################################
//! A module of widgets to control the image processing pipeline
namespace tp_qt_pipeline_widgets
{

using PipelineErrors = std::unordered_map<tp_pipeline::StepDetails*, std::vector<std::string>>;

//##################################################################################################
const char* defaultNodesStyle();

//##################################################################################################
void configureQtNodesStyle(const tp_data::CollectionFactory& collectionFactory);

}
