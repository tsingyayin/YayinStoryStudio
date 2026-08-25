#ifndef Visindigo_General_private_VersionMacro_h
#define Visindigo_General_private_VersionMacro_h

// Single source of truth for the Visindigo version numbers.
// This header contains ONLY preprocessor macros and does NOT depend on any
// Qt header, so it can be included directly by rc.exe (to generate the
// version resource) as well as by C++ code.
//
// The build number (Visindigo_VERSION_BUILD) and the full version string
// (Visindigo_VERSION_STRING) are generated into General/private/AUTO_VERSION.h
// by pytools/AutoVersionFromGIT.py on every build.

// Visindigo version macro
#define Visindigo_VERSION_MAJOR 0
#define Visindigo_VERSION_MINOR 17
#define Visindigo_VERSION_PATCH 0

#define Visindigo_ABI_VERSION_MAJOR 17
#define Visindigo_ABI_VERSION_MINOR 0

#endif // Visindigo_General_private_VersionMacro_h
