#ifndef YSSCore_YSSCoreCompileMacro_h
#define YSSCore_YSSCoreCompileMacro_h
#include <QtCore/qtypes.h>

#ifdef _MSC_VER
#define YSSCoreAPI __declspec(dllimport)
#ifdef YSSCore_DEVELOPMENT
#undef YSSCoreAPI
#define YSSCoreAPI
#endif
#ifdef YSSCore_EXPORT
#undef YSSCoreAPI
#define YSSCoreAPI __declspec(dllexport)
#endif
#else
#define YSSCoreAPI
#endif

#include <VIMacro.h>
#endif // YSSCore_YSSCoreCompileMacro_h
