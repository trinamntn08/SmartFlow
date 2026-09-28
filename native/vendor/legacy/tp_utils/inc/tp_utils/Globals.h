#pragma once

#include "lib_platform/Warnings.h" // IWYU pragma: keep
#include "lib_platform/RandomDevice.h"
#include "lib_platform/NotNull.h"
#include "lib_platform/Globals.h"

#include <functional>

#include <string>
#include <vector>
#include <list>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

#if defined(TP_UTILS_LIBRARY)
#  define TP_UTILS_EXPORT TP_EXPORT
#else
#  define TP_UTILS_EXPORT TP_IMPORT
#endif

//##################################################################################################
//TP_UNUSED
#define TP_UNUSED(var) (void)(var)

//##################################################################################################
//TP_NONCOPYABLE
#define TP_NONCOPYABLE(T) \
  T(const T&)=delete; \
  T(const T &&)=delete; \
  T& operator=(const T&)=delete; \
  T(T&&)=delete; \
  T& operator=(T&&)=delete

//##################################################################################################
//TP_NODISCARD
#ifdef TP_WIN32
#  define TP_NODISCARD
#else
#  define TP_NODISCARD __attribute__((warn_unused_result))
#endif

//##################################################################################################
//TP_DEPRECATED
#if __cplusplus > 201402L
#define TP_DEPRECATED(typ, func) [[deprecated]] typ func
#else
#ifdef __GNUC__
#define TP_DEPRECATED(typ, func) typ func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define TP_DEPRECATED(typ, func) typ __declspec(deprecated) func
#else
#define TP_DEPRECATED(typ, func) typ func
#endif
#endif


//##################################################################################################
#if __cplusplus >= 202002L
#define TP_LIKELY(x)      (x)[[likely]]
#define TP_UNLIKELY(x)    (x)[[unlikely]]
#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define TP_LIKELY(x)      (__builtin_expect(!!(x), 1))
#define TP_UNLIKELY(x)    (__builtin_expect(!!(x), 0))
#else
#define TP_LIKELY(x)      (x)
#define TP_UNLIKELY(x)    (x)
#endif

//##################################################################################################
//TP_SIZEOF
template<int s> struct __TP_SIZEOF;
//! Use this to show the size of objects at compile time, will fail at the line it is used.
#define TP_SIZEOF(o)__TP_SIZEOF<sizeof(o)> __TP_sizeof;

//##################################################################################################
//TP_LINE_STRING
#define TP_STRINGIZE(x) TP_STRINGIZE2(x)
#define TP_STRINGIZE2(x) #x
#define TP_LINE_STRING TP_STRINGIZE(__LINE__)

//##################################################################################################
//TP_CONCAT
#define TP_CONCAT_(s1, s2) s1##s2
#define TP_CONCAT(s1, s2) TP_CONCAT_(s1, s2)

//##################################################################################################
//TP_INIT
//! Scoped initialization.
#define TP_INIT(init) TPInit TP_CONCAT(tpInit, __LINE__){(init)}

//##################################################################################################
//TP_CLEANUP
//! Scoped cleanup.
#define TP_CLEANUP(cleanup) TPCleanUp TP_CONCAT(tpCleanUp, __LINE__){(cleanup)}

//##################################################################################################
//TP_INIT_AND_CLEANUP
//! Scoped initialization and cleanup.
#define TP_INIT_AND_CLEANUP(init, cleanup) TPInitAndCleanup TP_CONCAT(tpInitAndCleanup, __LINE__){(init), (cleanup)}

#define TP_DEFINE_FLAGS(C) \
  inline C operator|(C lhs, C rhs) \
{ \
  using T = std::underlying_type_t <C>; \
  return static_cast<C>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
  } \
  inline C& operator |= (C& lhs, C rhs) \
{ \
  lhs = lhs | rhs; \
  return lhs; \
  } \
  inline bool operator&(C lhs, C rhs) \
{ \
  using T = std::underlying_type_t <C>; \
  return (static_cast<T>(lhs) & static_cast<T>(rhs))!=0; \
  } \
  struct TP_CONCAT(TP_DEFINE_FLAGS_, __LINE__)

//##################################################################################################
//! Return a const object
template<typename T>
const T& tpConst(const T& o){return o;}
template<typename T>
const T* tpConst(const T* o){return o;}
template<typename T>
const T* tpConst(T* o){return o;}
template<typename T>
const T& tpConst(const T&& o)=delete;

//##################################################################################################
template<typename T>
T tpBound(T l, T v, T h)
{
  return v<l?l:(v>h?h:v);
}

//##################################################################################################
template<typename T>
T tpMin(T a, T b)
{
  return a<b?a:b;
}

//##################################################################################################
template<typename T>
T tpMax(T a, T b)
{
  return a>b?a:b;
}

//##################################################################################################
std::string TP_UTILS_EXPORT tpFromHEX(const std::string& input);

//##################################################################################################
std::string TP_UTILS_EXPORT tpToHex(const std::string& input);

//##################################################################################################
//! Returns true if input starts with the string in s
bool TP_UTILS_EXPORT tpStartsWith(const std::string& input, const std::string& s);

//##################################################################################################
//! Returns true if input ends with the string in s
bool TP_UTILS_EXPORT tpEndsWith(const std::string& input, const std::string& s);

//##################################################################################################
std::string tpToLower(const std::string& str);

//##################################################################################################
std::string tpToUpper(const std::string& str);

//##################################################################################################
std::string tpTrim(const std::string& str, const std::string& whitespace = " \n\r\t");

//##################################################################################################
//! Returns true if input contains the string in s
bool TP_UTILS_EXPORT tpContains(const std::string& input, const std::string& s);

//##################################################################################################
//! Returns true if input contains the string in s
bool TP_UTILS_EXPORT tpContains(const std::string& input, const char* s);

//##################################################################################################
template<class B, class E>
void tpRandomShuffle(B begin, E end)
{
  lib_platform::RandomDevice rd;
  std::mt19937 g(rd());
  std::shuffle(begin, end, g);
}

//##################################################################################################
void TP_UTILS_EXPORT* tpVoidLiteral(size_t value);

//##################################################################################################
enum class TPSplitBehavior
{
  KeepEmptyParts,
  SkipEmptyParts
};

//##################################################################################################
enum class TPCreateFullPath
{
  Yes,
  No
};

//##################################################################################################
enum class TPRecursive
{
  Yes,
  No
};

//##################################################################################################
enum class TPAppend
{
  Yes,
  No
};

//##################################################################################################
enum class TPReadOnly
{
  Yes,
  No
};

//##################################################################################################
//! Split a string on a delimiter
void TP_UTILS_EXPORT tpSplit(std::vector<std::string>& result,
                             const std::string& input,
                             const std::string& del,
                             TPSplitBehavior behavior=TPSplitBehavior::KeepEmptyParts);

//##################################################################################################
//! Split a string on a delimiter
void TP_UTILS_EXPORT tpSplit(std::vector<std::string>& result,
                             const std::string& input,
                             char del,
                             TPSplitBehavior behavior=TPSplitBehavior::KeepEmptyParts);

//##################################################################################################
//! Remove all instances of a character from a string.
void TP_UTILS_EXPORT tpRemoveChar(std::string& s, char c);

//##################################################################################################
template<typename T>
void tpDeleteAll(const T& container)
{
  for(auto i : container)
    delete i;
}

//##################################################################################################
template<typename T>
typename std::list<T>::value_type tpTakeLast(std::list<T>& container)
{
  typename std::list<T>::value_type t;
  std::swap(t, container.back());
  container.pop_back();
  return t;
}

//##################################################################################################
template<typename T>
typename std::vector<T>::value_type tpTakeLast(std::vector<T>& container)
{
  typename std::vector<T>::value_type t;
  std::swap(t, container.back());
  container.pop_back();
  return t;
}

//##################################################################################################
template<typename T>
typename T::value_type tpTakeFirst(T& container)
{
  auto i = container.begin();
  typename T::value_type t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename T, typename I>
typename T::value_type tpTakeAt(T& container, I index)
{
  auto i = container.begin() + index;
  typename T::value_type t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename T>
bool tpContainsKey(const T& container, const typename T::key_type& key)
{
  return container.count(key)!=0;
}

//##################################################################################################
template<typename T>
bool tpContainsValue(const T& container, const typename T::mapped_type& value)
{
  return (container.find(value) != container.end());
}

//##################################################################################################
template<typename V>
bool tpContains(const std::vector<V>& vec, const V& value)
{
  // std::vector doesn't have find() method, so we need to use std::find() instead
  return (std::find(vec.begin(), vec.end(), value) != vec.end());
}

//##################################################################################################
template<typename V>
bool tpAny(const std::vector<V>& curr, const std::vector<V>& ref)
{
  return std::find_first_of(curr.begin(), curr.end(), ref.begin(), ref.end()) != curr.end();
}

//##################################################################################################
template<typename V>
bool tpAny(const std::vector<V>& curr, const std::initializer_list<V>& ref)
{
  return std::find_first_of(curr.begin(), curr.end(), ref.begin(), ref.end()) != curr.end();
}

//##################################################################################################
template<typename V, typename T>
bool tpContains(const std::vector<T>& vec, const V& value)
{
  // std::vector doesn't have find() method, so we need to use std::find() instead
  return (std::find(vec.begin(), vec.end(), value) != vec.end());
}

//##################################################################################################
template<typename V, typename T>
bool tpContains(const T& container, const V& value)
{
  // for containers with built-in find() method, because this is much faster than std::find()
  return (container.find(value) != container.end());
}

//##################################################################################################
template<typename T, typename V>
void tpRemoveOne(T& container, const V& value)
{
  // for containers with built-in find() method, because this is much faster than std::find()
  auto i = container.find(value);
  if(i != container.end())
    container.erase(i);
}

//##################################################################################################
template<typename V>
void tpRemoveOne(std::vector<V>& vec, const V& value)
{
  // std::vector doesn't have find() method, so we need to use std::find() instead
  auto i = std::find(vec.begin(), vec.end(), value);
  if(i != vec.end())
    vec.erase(i);
}

//##################################################################################################
template<typename T>
void tpRemoveAll(T& container, const typename T::value_type& value)
{
  // for containers with built-in find() method, because this is much faster than std::find()
  auto i = container.find(value);
  while(i != container.end())
  {
    container.erase(i);
    i = container.find(value);
  }
}

//##################################################################################################
template<typename V>
void tpRemoveAll(std::vector<V>& vec, const V& value)
{
  // std::vector doesn't have find() method, so we need to use std::find() instead
  auto i = std::find(vec.begin(), vec.end(), value);
  while(i != vec.end())
  {
    vec.erase(i);
    i = std::find(vec.begin(), vec.end(), value);
  }
}

//##################################################################################################
template<typename T, typename I>
void tpRemoveAt(T& container, I index)
{
  container.erase(container.begin() + index);
}

//##################################################################################################
template<typename T, typename V>
size_t tpIndexOf(const T& container, const V& value)
{
  // for containers with built-in find() method, because this is much faster than std::find()
  return size_t(container.find(value) - container.begin());
}

//##################################################################################################
template<typename V>
size_t tpIndexOf(const std::vector<V>& vec, const V& value)
{
  // std::vector doesn't have find() method, so we need to use std::find instead
  return size_t(std::find(vec.begin(), vec.end(), value) - vec.begin());
}

//##################################################################################################
template<typename V, size_t N>
size_t tpIndexOf(const std::array<V,N>& arr, const V& value)
{
  // std::array doesn't have find() method, so we need to use std::find instead
  return size_t(std::find(arr.begin(), arr.end(), value) - arr.begin());
}

//##################################################################################################
template<typename T, typename I>
void tpInsert(T& container, I index, const typename T::value_type& value)
{
  container.insert(container.begin() + int(index), value);
}

//##################################################################################################
template<typename T, typename I>
void tpReplace(T& container, I index, const typename T::value_type& value)
{
  container.at(index) = value;
}

//##################################################################################################
template<typename T>
typename T::mapped_type tpGetMapValue(const T& map,
                                      const typename T::key_type& key,
                                      const typename T::mapped_type& defaultValue=typename T::mapped_type())
{
  auto i = map.find(key);
  return (i != map.end())?(i->second):defaultValue;
}

//##################################################################################################
template<typename T>
typename T::key_type tpGetMapKey(const T& map,
                                 const typename T::mapped_type& value,
                                 const typename T::key_type& defaultKey=typename T::key_type())
{
  for(const auto& i : map)
    if(i.second == value)
      return i.first;
  return defaultKey;
}

//##################################################################################################
template<typename T>
typename std::vector<typename T::key_type> tpKeys(const T& map)
{
  std::vector<typename T::key_type> keys;
  for(const auto& i : map)
    keys.push_back(i.first);
  return keys;
}

//##################################################################################################
std::string TP_UTILS_EXPORT tpToUTF8(const std::u16string& source);

//##################################################################################################
std::string TP_UTILS_EXPORT tpToUTF8(const std::wstring& source);

//##################################################################################################
std::u16string TP_UTILS_EXPORT tpFromUTF8(const std::string& source);

//##################################################################################################
std::wstring tpWStringFromUTF8(const std::string& source);

//##################################################################################################
template<typename T>
bool tpNumber(const std::string& s, T& n, T d=T())
{
  try
  {
    if constexpr (std::is_same_v<T, int>)
        n = std::stoi(s);
    else if constexpr (std::is_same_v<T, long>)
        n = std::stol(s);
    else if constexpr (std::is_same_v<T, unsigned long>)
        n = std::stoul(s);
    else if constexpr (std::is_same_v<T, long long>)
        n = std::stoll(s);
    else if constexpr (std::is_same_v<T, unsigned long long>)
        n = std::stoull(s);
    else if constexpr (std::is_same_v<T, float>)
        n = std::stof(s);
    else if constexpr (std::is_same_v<T, double>)
        n = std::stod(s);
    else if constexpr (std::is_same_v<T, long double>)
        n = std::stold(s);
    else
    n = T(std::stoi(s));
    return true;
  }
  catch(const std::invalid_argument&)
  {
  }
  catch(const std::out_of_range&)
  {
  }

  n=d;
  return false;
}

template<typename T, typename J>
T tpConstructFromJSON(const J& j)
{
  T s;
  s.loadState(j);
  return s;
}

namespace detail
{
//##################################################################################################
template <typename T, size_t...Is>
std::array<T, sizeof...(Is)> tpMakeArray(const T& value, std::index_sequence<Is...>)
{
  return {{(static_cast<void>(Is), value)...}};
}
}

//##################################################################################################
template <typename T, size_t N>
std::array<T, N> tpMakeArray(const T& value)
{
  return detail::tpMakeArray(value, std::make_index_sequence<N>());
}

//##################################################################################################
template<typename T>
inline bool tpUpdateIfChanged(T& value, const T& newValue)
{
  if(value != newValue)
  {
    value = newValue;
    return true;
  }
  return false;
}

//##################################################################################################
template<typename T>
class TPCleanUp
{
  TP_NONCOPYABLE(TPCleanUp);
  T m_cleanup;
public:
  TPCleanUp(const T& cleanup):m_cleanup(cleanup){}
  ~TPCleanUp(){m_cleanup();}
};

//##################################################################################################
class TPInitAndCleanup
{
  TP_NONCOPYABLE(TPInitAndCleanup);
  std::function<void()> m_cleanup;
public:
  TPInitAndCleanup(const std::function<void()>& init, const std::function<void()>& cleanup):
    m_cleanup(cleanup)
  {
    init();
  }
  ~TPInitAndCleanup(){m_cleanup();}
};

//##################################################################################################
class TPInit
{
  TP_NONCOPYABLE(TPInit);
public:
  TPInit(const std::function<void()>& init)
  {
    init();
  }
};

//##################################################################################################
//! Classes used throughout the rest of tp Toolkit.
/*!
This module provides a set of general purpose classes that form the core of tp Toolkit.
*/
namespace tp_utils
{
namespace detail
{
//##################################################################################################
int& staticInit();
}
//##################################################################################################
int staticInit();

//##################################################################################################
void TP_UTILS_EXPORT leftJustified(std::string& text, size_t maxLength, char padding=' ');

//##################################################################################################
void TP_UTILS_EXPORT rightJustified(std::string& text, size_t maxLength, char padding=' ');

//##################################################################################################
std::string TP_UTILS_EXPORT fixedWidthKeepRight(std::string data, size_t len, char pad);

//##################################################################################################
std::string TP_UTILS_EXPORT fixedWidthKeepLeft(std::string data, size_t len, char pad);

//##################################################################################################
//! Replace key with value in result.
void replace(std::string& result, const std::string& key, const std::string& value);

//##################################################################################################
std::string replaced(std::string input, const std::string& key, const std::string& value);

//##################################################################################################
std::string formatFloat(float value, int precision);

//##################################################################################################
std::string formatDouble(double value, int precision);

//##################################################################################################
//! Format a number as B, kB, MB, ...
std::string formatSizeBytes(int64_t sizeInBytes, bool alignRight=false);

//##################################################################################################
//! Format a number as B, KiB, MiB, ...
std::string formatSizeBinaryBytes(int64_t sizeInBytes, bool alignRight=false);

//##################################################################################################
bool TP_UTILS_EXPORT parseColor(const std::string& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

//##################################################################################################
bool TP_UTILS_EXPORT parseColorF(const std::string& color, float& r, float& g, float& b, float& a);

}

#ifdef __clang__
#pragma clang diagnostic push
#if !defined(__has_warning) || __has_warning("-Wnon-template-friend")
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

namespace detail
{
namespace SelfType
{
template <typename T>
struct Reader
{
  friend auto adl_GetSelfType(Reader<T>);
};

template <typename T, typename U>
struct Writer
{
  friend auto adl_GetSelfType(Reader<T>){return U{};}
};

inline void adl_GetSelfType() {}

template <typename T>
using Read = std::remove_pointer_t<decltype(adl_GetSelfType(Reader<T>{}))>;
}
}

#define TP_D \
  private: \
  struct Private; \
  friend struct Private; \
  Private* d

#define TP_Q \
  private: \
  struct _self_type_tag {}; \
  constexpr auto _self_type_helper() -> decltype(::detail::SelfType::Writer<_self_type_tag, decltype(this)>{}, void()) {} \
  using Q = ::detail::SelfType::Read<_self_type_tag>

#define TP_DQ \
  TP_D;\
  TP_Q

#ifdef TP_ENABLE_PRINT_FUNCTION_NAME
namespace detail
{
extern bool tpPrintFunctionNames;
}

//##################################################################################################
#define tpEnablePrintFunctionNames(enable) detail::tpPrintFunctionNames = enable;

//##################################################################################################
void PRINT_FUNCTION_NAME(const std::string& name);
#else

//##################################################################################################
#define tpEnablePrintFunctionNames(enable)

//##################################################################################################
#define PRINT_FUNCTION_NAME(A)

#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace tp_utils
{
class AbstractCrossThreadCallbackFactory;
}
using TPCTCFactory = cpp::not_null<tp_utils::AbstractCrossThreadCallbackFactory*>;
