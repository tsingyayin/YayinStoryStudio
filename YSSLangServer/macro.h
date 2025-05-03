#pragma once

#define YSSAPI __declspec(dllimport)
#ifdef YSS_DEVELOPMENT
#undef YSSAPI
#define YSSAPI
#endif
#ifdef YSS_EXPORT_DLL
#undef YSSAPI
#define YSSAPI __declspec(dllexport)
#endif
