#pragma once

#include "tp_utils/Globals.h"

namespace tp_utils
{

typedef void* WeakStringID;
struct StaticStringID;

//##################################################################################################
//! A class that implements efficent string based identifiers
/*!
This is probably the most important class in tp Toolkit, a majority of the other components are
linked together by strings, and this class is used to make this efficent.
*/
class  TP_UTILS_EXPORT StringID final
{
  friend bool operator==(const StringID& a, const StringID& b);
  friend bool operator!=(const StringID& a, const StringID& b);
  friend bool operator==(const StaticStringID& a, const StaticStringID& b);
  friend bool operator!=(const StaticStringID& a, const StaticStringID& b);
  friend struct std::hash<tp_utils::StringID>;
  friend struct std::hash<tp_utils::StaticStringID>;

public:
  //################################################################################################
  //! Construct an invalid/blank string id
  StringID();

  //################################################################################################
  //! Copy another string id
  StringID(const StringID& other);

  //################################################################################################
  //! Move another string id
  StringID(StringID&& other) noexcept;

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TP_DECLARE_ID \endlink and \link TP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.
  \param string - The string to generate the StringID from.
  */
  StringID(const std::string& string);

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TP_DECLARE_ID \endlink and \link TP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.

  \param string - The null terminated string to generate the StringID from.
  */
  StringID(const char* string);

  //################################################################################################
  //! Copy another StringID
  StringID& operator=(const StringID& other);

  //################################################################################################
  //! Move another StringID
  StringID& operator=(StringID&& other) noexcept;

  //################################################################################################
  //! Copy another string
  StringID& operator=(const char* string);

  //################################################################################################
  //! Copy another string
  StringID& operator=(const std::string& string);

  //################################################################################################
  //! Decrement the reference count and clean up
  ~StringID();

  //################################################################################################
  //! An instance of the StringID must remain valid for the life of the WeakStringID.
  /*!
  Using TP_DEFINE_ID will ensure that the StringID remains valid.
  */
  WeakStringID weak() const
  {
    return sd;
  }

  //################################################################################################
  static StringID fromWeak(WeakStringID weak);

  //################################################################################################
  static std::vector<WeakStringID> toWeak(const std::vector<StringID>& ids);

  //################################################################################################
  //! Returns the string that this StringID represents
  /*!
  \return The string or an empty string id this is an invalid StringID.
  */
  const std::string& toString() const;

  //################################################################################################
  //! Returns true if this points to a valid key
  bool isValid() const;

  //################################################################################################
  void reset();

  //################################################################################################
  static std::vector<std::string> toStringList(const std::vector<StringID>& stringIDs);

  //################################################################################################
  static std::vector<StringID> fromStringList(const std::vector<std::string>& stringIDs);

private:
  //################################################################################################
  void fromString(const std::string& string);

  //################################################################################################
  void attach();

  //################################################################################################
  void detach();

  //################################################################################################
  void silentDetach();

  //################################################################################################
  void silentDetachInternal();

  struct SharedData;
  SharedData* sd;
  friend struct SharedData;

  struct StaticData;
  static StaticData& staticData(size_t hash);
  friend struct StaticData;
  friend struct StaticStringID;
};

//##################################################################################################
inline bool operator==(const StringID& a, const StringID& b)
{
  return (a.sd == b.sd);
}

//##################################################################################################
inline bool operator!=(const StringID& a, const StringID& b)
{
  return (a.sd != b.sd);
}

//##################################################################################################
//! Used for sorting StringID's
/*!
<pre>
std::vector<tp_utils::StringID> listOfIDs;
qSort(listOfIDs.begin(), listOfIDs.end(), tp_utils::lessThanStringID);
</pre>

\param lhs - Left hand side of less than
\param rhs - Right hand side of less than
\return true if lhs is less than rhs
*/
bool TP_UTILS_EXPORT lessThanStringID(const StringID& lhs, const StringID& rhs);

//##################################################################################################
//! Concatenate a list of StringID key strings.
/*!
\param ids - List of StringIDs to concatenate.
\return  Concatenated string, comma separated.
*/
std::string TP_UTILS_EXPORT join(const std::vector<StringID>& ids);

//##################################################################################################
std::string TP_UTILS_EXPORT join(const std::vector<StringID>& ids, const std::string& del);

//##################################################################################################
std::string TP_UTILS_EXPORT join(const std::vector<std::string>& parts, const std::string& del);

//##################################################################################################
struct StaticStringID
{
  StringID sid;

  //################################################################################################
  StaticStringID() = default;

  //################################################################################################
  StaticStringID(const StaticStringID& other) = default;

  //################################################################################################
  //! Move another string id
  StaticStringID(StaticStringID&& other) noexcept = default;

  //################################################################################################
  StaticStringID(const char* string):
    sid(string)
  {

  }

  //################################################################################################
  StaticStringID(const std::string& string):
    sid(string)
  {

  }

  //################################################################################################
  ~StaticStringID()
  {
    sid.silentDetach();
  }

  //################################################################################################
  StaticStringID& operator=(const StaticStringID& other) = default;

  //################################################################################################
  StaticStringID& operator=(StaticStringID&& other) noexcept = default;
};

//##################################################################################################
inline bool operator==(const StaticStringID& a, const StaticStringID& b)
{
  return (a.sid.sd == b.sid.sd);
}

//##################################################################################################
inline bool operator!=(const StaticStringID& a, const StaticStringID& b)
{
  return (a.sid.sd != b.sid.sd);
}

//##################################################################################################
[[nodiscard]] tp_utils::StringID makeUUID();

}

namespace std
{
template <>
struct hash<tp_utils::StringID>
{
  size_t operator()(const tp_utils::StringID& stringID) const
  {
    return hash<void*>()(stringID.sd);
  }
};

template <>
struct hash<tp_utils::StaticStringID>
{
  size_t operator()(const tp_utils::StaticStringID& stringID) const
  {
    return hash<void*>()(stringID.sid.sd);
  }
};
template <>
struct hash<std::vector<tp_utils::StringID>>
{
  size_t operator()(const std::vector<tp_utils::StringID>& stringIDs) const
  {
    auto h = hash<void*>()(nullptr);
    for(const auto& stringID : stringIDs)
      h ^= std::hash<tp_utils::StringID>()(stringID) + 0x9e3779b9 + (h<<6) + (h>>2);
    return h;
  }
};
template <>
struct hash<std::vector<tp_utils::WeakStringID>>
{
  size_t operator()(const std::vector<tp_utils::WeakStringID>& stringIDs) const
  {
    auto h = hash<void*>()(nullptr);
    for(const auto& stringID : stringIDs)
      h ^= std::hash<tp_utils::WeakStringID>()(stringID) + 0x9e3779b9 + (h<<6) + (h>>2);
    return h;
  }
};
}

//##################################################################################################
//! Declare a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TP_DECLARE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TP_DECLARE_ID(methodName, idString) const tp_utils::StringID& methodName()

//##################################################################################################
//! Define a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TP_DEFINE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TP_DEFINE_ID(methodName, idString)            \
  const tp_utils::StringID& methodName() {            \
  static const tp_utils::StaticStringID id(idString); \
  return id.sid;                                      \
  }                                                   \
  void ANONYMOUS_FUNCTION()

//##################################################################################################
//! Split a list of StringID's.
void TP_UTILS_EXPORT tpSplitSIDs(std::vector<tp_utils::StringID>& result, const std::string& input);

#include "tp_utils/Globals.h"
