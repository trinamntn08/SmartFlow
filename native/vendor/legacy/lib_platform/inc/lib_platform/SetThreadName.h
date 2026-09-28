#ifndef lib_platform_SetThreadName_h
#define lib_platform_SetThreadName_h

#include "lib_platform/Globals.h"

#include <thread>
#include <string>

namespace lib_platform
{
//##################################################################################################
//! Set the name of the calling thread.
void LIB_PLATFORM_EXPORT setThreadName(const std::string& threadName);

//##################################################################################################
//! set the name of a thread.
void LIB_PLATFORM_EXPORT setThreadName(std::thread& thread, const std::string& threadName);
}

#endif
