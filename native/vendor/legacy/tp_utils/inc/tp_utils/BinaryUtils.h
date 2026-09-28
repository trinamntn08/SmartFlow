#pragma once

#include "tp_utils/Globals.h"

namespace tp_utils
{
//##################################################################################################
//! Converts binary data into a format that prints nicely.
/*!
This will replace all non printable characters with spaces and then return the results along with
the original size, the number of characters that are printed can be limited with maxLen.

\param data - The source data.
\param maxLen - The maximum number of characters.

\returns - A printable version of the data surrounded by brackets and prefixed with its length.
*/
std::string TP_UTILS_EXPORT binaryDebug(const std::string& data, size_t maxLen = 30, size_t maxLenBinary=0);

//##################################################################################################
std::string TP_UTILS_EXPORT binaryNumericDebug(const std::string& data, size_t maxLen = 30);

//##################################################################################################
std::string TP_UTILS_EXPORT summaryBinaryDebug(const std::string& data);


}
