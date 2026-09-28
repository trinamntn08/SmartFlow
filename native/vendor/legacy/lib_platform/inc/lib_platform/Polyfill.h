
#ifdef TP_ANDROID
#include "lib_platform/Android.h"
#elif defined TP_OSX
#include "lib_platform/OSX.h"
#else
#define TP_ENUM_HASH(c)
#endif
