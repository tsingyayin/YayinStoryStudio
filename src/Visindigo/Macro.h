#pragma once

#define VisindigoAPI __declspec(dllimport)
#ifdef Visindigo_DEVELOPMENT
#undef VisindigoAPI
#define VisindigoAPI
#endif
#ifdef Visindigo_EXPORT
#undef VisindigoAPI
#define VisindigoAPI __declspec(dllexport)
#endif

#include "VIMacro.h"