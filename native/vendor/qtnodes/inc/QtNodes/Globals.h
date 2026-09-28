#pragma once

#include <QtCore/qglobal.h>

#if defined(SMARTFLOW_QTNODES_STATIC)
#  define LIB_QTNODES_SHARED_EXPORT
#elif defined(LIB_QTNODES_LIBRARY)
#  define LIB_QTNODES_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIB_QTNODES_SHARED_EXPORT Q_DECL_IMPORT
#endif

//##################################################################################################
//! Widgets for rendering nodes
namespace lib_qtnodes
{

//##################################################################################################
const char* defaultStyle();

}

