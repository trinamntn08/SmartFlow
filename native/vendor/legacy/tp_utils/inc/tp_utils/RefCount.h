#ifndef tp_utils_RefCount_h
#define tp_utils_RefCount_h

#include "tp_utils/Globals.h"

#ifdef TP_REF_COUNT
#include "tp_utils/StringID.h"

#include <unordered_map>
#include <map>

namespace tp_utils
{

//################################################################################################
//! This holds the count for a single type
struct TP_UTILS_EXPORT InstanceDetails
{
  int count;      //!< The current count of live objects.
  int peak;       //!< The peak count of living objects.
  int64_t total;  //!< The total count including instances that have been deleted.

  InstanceDetails():
    count(0),
    peak(0),
    total(0)
  {}
};

//##################################################################################################
//! A class for counting instances of different types
/*!
This class is used as a debug tool to count the number of instances of different types as the
program runs. To count the number of instances of any given class call \link #TP_REF(type) \endlink
in the constructor and \link #TP_UNREF(type) \endlink in the destructor, do not call ref() and
unref() directly. The type should be the namespace::class of your class.
*/
class TP_UTILS_EXPORT RefCount
{
  RefCount(){}
public:
  //################################################################################################
  //! Increments the count for type
  /*!
  \warning Don't call this directly, use TP_UNREF
  \param type - The type to increment
  */
  static void ref(const tp_utils::StaticStringID& type);

  //################################################################################################
  //! Decrements the count for type
  /*!
  \warning Don't call this directly, use TP_UNREF
  \param type - The type to decrement
  */
  static void unref(const tp_utils::StaticStringID& type);

  //################################################################################################
  //! Lock the instances hash
  /*!
  You must call this before using instances() and then call unlock() once you are finished with
  instances.
  */
  static void lock();

  //################################################################################################
  //! Unlock the instances hash
  static void unlock();

  //################################################################################################
  //! Return the instance counts
  /*!
  \warning you must lock before using this
  \return A hash of type to count
  */
  static const std::unordered_map<tp_utils::StaticStringID, InstanceDetails>& instances();

  //################################################################################################
  static std::vector<std::string> serialize();

  //################################################################################################
  static std::string takeResults();

  //################################################################################################
  static std::map<std::string, size_t> keyValueResults();
};

//##################################################################################################
void refCountHistoryReport();

}

//##################################################################################################
#define TP_REF_COUNT_OBJECTS(type)\
  class _TP_REF_COUNT_OBJECT_\
  {\
  static const tp_utils::StaticStringID& getType()\
  {\
  static const tp_utils::StaticStringID typeSID(std::string(type));\
  return typeSID;\
  }\
  const tp_utils::StaticStringID& m_type{getType()};\
  public:\
  _TP_REF_COUNT_OBJECT_ (){tp_utils::RefCount::ref  (m_type);}\
  _TP_REF_COUNT_OBJECT_ (const _TP_REF_COUNT_OBJECT_& other){TP_UNUSED(other);tp_utils::RefCount::ref  (m_type);}\
  ~_TP_REF_COUNT_OBJECT_(){tp_utils::RefCount::unref(m_type);}\
  _TP_REF_COUNT_OBJECT_& operator=(const _TP_REF_COUNT_OBJECT_&)=delete;\
  _TP_REF_COUNT_OBJECT_& operator=(_TP_REF_COUNT_OBJECT_&&)=delete;\
  _TP_REF_COUNT_OBJECT_ (_TP_REF_COUNT_OBJECT_&&)=delete;\
  }; \
  void swap(_TP_REF_COUNT_OBJECT_&, _TP_REF_COUNT_OBJECT_&) noexcept{} \
  _TP_REF_COUNT_OBJECT_ _TP_REF_COUNT_OBJECT_instance


//##################################################################################################
//! Increment the ref count for type
/*!
\def TP_REF(type)
\param type - The type of the class to ref count as a string
*/
#define TP_REF(type){\
  static const tp_utils::StaticStringID TP_REF_TYPE(type);\
  tp_utils::RefCount::ref(TP_REF_TYPE);}do{}while(0)


//##################################################################################################
//! Increment the ref count for type
/*!
\def TP_UNREF(type)
\param type - The type of the class to ref count as a string
*/
#define TP_UNREF(type){\
  static const tp_utils::StaticStringID TP_REF_TYPE(type);\
  tp_utils::RefCount::unref(TP_REF_TYPE);}do{}while(0)

#else

#define TP_REF_COUNT_OBJECTS(type) struct TP_REF_COUNT_OBJECTS_
#define TP_REF(type) do{}while(0)
#define TP_UNREF(type) do{}while(0)

#endif

#endif
