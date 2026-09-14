#ifndef VIAgent_VIAgentCompileMacro_h
#define VIAgent_VIAgentCompileMacro_h
#include <VICompileMacro.h>

#ifdef _MSC_VER
#define VIAgentAPI Q_DECL_IMPORT
#ifdef VIAgent_DEVELOPMENT
#undef VIAgentAPI
#define VIAgentAPI
#endif
#ifdef VIAgent_EXPORT
#undef VIAgentAPI
#define VIAgentAPI Q_DECL_EXPORT
#endif
#else
#define VIAgentAPI
#endif
#endif // VIAgent_VIAgentCompileMacro_h
