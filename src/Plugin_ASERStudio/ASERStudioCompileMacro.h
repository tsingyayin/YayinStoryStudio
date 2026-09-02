#ifndef Plugin_ASERStudio_ASERStudioCompileMacro_h
#define Plugin_ASERStudio_ASERStudioCompileMacro_h
#define ASERAPI __declspec(dllimport)
#ifdef ASER_DEVELOPMENT
#undef ASERAPI
#define ASERyAPI
#endif
#ifdef ASER_EXPORT
#undef ASERAPI
#define ASERAPI __declspec(dllexport)
#endif
#endif // Plugin_ASERStudio_ASERStudioCompileMacro_h
