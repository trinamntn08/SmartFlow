#ifndef lib_platform_Android_h
#define lib_platform_Android_h

//The code in this file is here simply to work arround issues in GCC 4.9 used in Android. The NDK
//has now switched over to Clang so this will not need to exist much longer.

#ifdef TP_ANDROID
#if(defined(__GNUC__) && !defined(__clang__))
#define TP_ANDROID_GCC_COMPILER
#endif
#endif

//PLATFORM_ABSTRACTIONS
#ifdef TP_ANDROID_GCC_COMPILER
#include <string>
#include <sstream>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>

struct lconv* localeconv(void);

namespace std
{
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

inline float strtof(const char* nptr, char** endptr)
{
  return ::strtof(nptr, endptr);
}

inline long double strtold(const char* __s, char** __end_ptr)
{
  return ::strtold(__s, __end_ptr);
}

inline unsigned long long strtoull(const char* __s, char** __end_ptr, int __base)
{
  return ::strtoull(__s, __end_ptr, __base);
}

inline long long strtoll(const char* __s, char** __end_ptr, int __base)
{
  return ::strtoll(__s, __end_ptr, __base);
}

inline int stoi(const string& str)
{
  if(str.empty())
    return 0;
  return ::atoi(str.data());
}

inline long long stoll(const string& str)
{
  if(str.empty())
    return 0;
  return ::strtoll(str.data(), 0, 10);
}

}

#define TP_ENUM_HASH(T) \
namespace std{template <>struct hash<T>{size_t operator()(const T& v) const \
{ \
  return hash<int>()(static_cast<int>(v)); \
}};}

#else
#define TP_ENUM_HASH(c)
extern int lib_platform_NoAndroid;
#endif

#endif
