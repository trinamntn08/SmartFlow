#ifndef lib_platform_Warnings_h
#define lib_platform_Warnings_h

#ifdef __clang__
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wundefined-func-template"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpadded"

//This little fudge gets round the Clang Code model pretending to be GCC issue in Qt Creator.
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wundefined-func-template"
#endif



#endif
