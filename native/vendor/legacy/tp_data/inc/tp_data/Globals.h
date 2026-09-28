#pragma once

#include "tp_utils/StringID.h"

#if defined(TP_DATA_LIBRARY)
#  define TP_DATA_SHARED_EXPORT TP_EXPORT
#else
#  define TP_DATA_SHARED_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! A pipeline for performing image processing
namespace tp_data
{
class CollectionFactory;

TP_DECLARE_ID(                       stringSID,                           "String");
TP_DECLARE_ID(                     stringIDSID,                        "String id");
TP_DECLARE_ID(                          intSID,                              "Int");
TP_DECLARE_ID(                        sizeTSID,                           "Size t");
TP_DECLARE_ID(                        floatSID,                            "Float");
TP_DECLARE_ID(                       doubleSID,                           "Double");
TP_DECLARE_ID(               stringIDVectorSID,                 "String id vector");
TP_DECLARE_ID(                    intVectorSID,                       "Int vector");
TP_DECLARE_ID(                  sizeTVectorSID,                    "Size t vector");
TP_DECLARE_ID(                  floatVectorSID,                     "Float vector");
TP_DECLARE_ID(                 doubleVectorSID,                    "Double vector");
TP_DECLARE_ID(                    intMatrixSID,                       "Int matrix");
TP_DECLARE_ID(                  sizeTMatrixSID,                    "Size t matrix");
TP_DECLARE_ID(                  floatMatrixSID,                     "Float matrix");
TP_DECLARE_ID(                 doubleMatrixSID,                    "Double matrix");
TP_DECLARE_ID(                         fileSID,                             "File");

//##################################################################################################
//! Add the collection factories that this module provides to the CollectionFactory
void createCollectionFactories(CollectionFactory& collectionFactory);

//##################################################################################################
//! Add all of the collection factories that have been registered to the CollectionFactory
void createAllCollectionFactories(CollectionFactory& collectionFactory);

//##################################################################################################
std::vector<std::function<void(CollectionFactory&)>>& createCollectionFactoriesRegister();

//##################################################################################################
//! Add this namspaces createCollectionFactories method to the global register.
#define REGISTER_CREATE_COLLECTION_FACTORIES \
extern int createCollectionFactories_reg; \
int createCollectionFactories_reg = [] \
{ \
  tp_data::createCollectionFactoriesRegister().emplace_back(createCollectionFactories); \
  return 0; \
}()

//##################################################################################################
//! Static initialization of this module, see TP_STATIC_INIT in dependencies.pri
int staticInit();

}
