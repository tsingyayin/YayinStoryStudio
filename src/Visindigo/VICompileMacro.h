#pragma once
#include <QtCompilerDetection>
#include <QtCore/qtypes.h>

#ifdef _MSC_VER
    #define VisindigoAPI Q_DECL_IMPORT
    #ifdef Visindigo_DEVELOPMENT
        #undef VisindigoAPI
        #define VisindigoAPI
    #endif
    #ifdef Visindigo_EXPORT
        #undef VisindigoAPI
    #define VisindigoAPI Q_DECL_EXPORT
    #endif
#else
#define VisindigoAPI
#endif

#include "VIMacro.h"