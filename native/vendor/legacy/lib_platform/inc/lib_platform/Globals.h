#pragma once

#include <stddef.h>

#ifdef TP_WIN32
#  ifdef TP_WIN32_STATIC
#    define TP_EXPORT
#    define TP_IMPORT
#  else
#    define TP_EXPORT __declspec(dllexport)
#    define TP_IMPORT __declspec(dllimport)
#  endif
#else
#  define TP_EXPORT
#  define TP_IMPORT
#endif

#if defined(LIB_PLATFORM_LIBRARY)
#  define LIB_PLATFORM_EXPORT TP_EXPORT
#else
#  define LIB_PLATFORM_EXPORT TP_IMPORT
#endif

#define LIB_PLATFORM_NONCOPYABLE(T) T(const T&)=delete; T& operator=(const T&)=delete; T(T&&)=delete; T& operator=(T&&)=delete

#ifndef TP_GIT_BRANCH
#define TP_GIT_BRANCH
#endif

#ifndef TP_GIT_COMMIT
#define TP_GIT_COMMIT
#endif

#ifndef TP_GIT_COMMIT_NUMBER
#define TP_GIT_COMMIT_NUMBER
#endif

#ifdef __cplusplus
  #if __cplusplus >= 201703L
    #define TP_FALLTHROUGH [[fallthrough]]
  #else
    #define TP_FALLTHROUGH
  #endif
#else  /* C */
  #if defined(__has_c_attribute)
    #if __has_c_attribute(fallthrough)
      #define TP_FALLTHROUGH [[fallthrough]]
    #else
      #define TP_FALLTHROUGH
    #endif
  #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    /* C23 */
    #define TP_FALLTHROUGH [[fallthrough]]
  #else
    #define TP_FALLTHROUGH
  #endif
#endif

#ifdef __cplusplus

//##################################################################################################
constexpr size_t operator""_KiB(unsigned long long int x)
{
  return 1024ULL * x;
}

//##################################################################################################
constexpr size_t operator""_MiB(unsigned long long int x)
{
  return 1024_KiB * x;
}

//##################################################################################################
constexpr size_t operator""_GiB(unsigned long long int x)
{
  return 1024_MiB * x;
}

//##################################################################################################
constexpr size_t operator""_TiB(unsigned long long int x)
{
  return 1024_GiB * x;
}

//##################################################################################################
constexpr size_t operator""_PiB(unsigned long long int x)
{
  return 1024_TiB * x;
}

//##################################################################################################
constexpr size_t operator""_KB(unsigned long long int x)
{
  return 1000ULL * x;
}

//##################################################################################################
constexpr size_t operator""_MB(unsigned long long int x)
{
  return 1000_KB * x;
}

//##################################################################################################
constexpr size_t operator""_GB(unsigned long long int x)
{
  return 1000_MB * x;
}

//##################################################################################################
constexpr size_t operator""_TB(unsigned long long int x)
{
  return 1000_GB * x;
}

//##################################################################################################
constexpr size_t operator""_PB(unsigned long long int x)
{
  return 1000_TB * x;
}

//##################################################################################################
//! Platform abstractions
namespace lib_platform
{
}

#endif

