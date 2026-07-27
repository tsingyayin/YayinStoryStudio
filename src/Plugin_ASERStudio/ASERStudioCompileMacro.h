#ifndef ASERStudio_ASERStudioCompileMacro_h
#define ASERStudio_ASERStudioCompileMacro_h
#define ASERAPI __declspec(dllimport)
#ifdef ASER_DEVELOPMENT
#undef ASERAPI
#define ASERyAPI
#endif
#ifdef ASER_EXPORT
#undef ASERAPI
#define ASERAPI __declspec(dllexport)
#endif
#endif // ASERStudio_ASERStudioCompileMacro_h
