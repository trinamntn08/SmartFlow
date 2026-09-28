//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//

#ifndef BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include "lib_platform/Globals.h"

#include <string>

#if defined(LIB_BASE64_LIBRARY)
#  define LIB_BASE64_EXPORT TP_EXPORT
#else
#  define LIB_BASE64_EXPORT TP_IMPORT
#endif

std::string LIB_BASE64_EXPORT base64_encode(unsigned char const* bytes_to_encode, size_t in_len);
std::string LIB_BASE64_EXPORT base64_encode(const std::string& s);

std::string LIB_BASE64_EXPORT base64_decode(const std::string& s);



#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */
