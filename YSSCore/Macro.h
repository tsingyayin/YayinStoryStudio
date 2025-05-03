#pragma once

#define YSSCoreAPI __declspec(dllimport)
#ifdef YSSCore_DEVELOPMENT
#undef YSSCoreAPI
#define YSSCoreAPI
#endif
#ifdef YSSCore_EXPORT
#undef YSSCoreAPI
#define YSSCoreAPI __declspec(dllexport)
#endif
