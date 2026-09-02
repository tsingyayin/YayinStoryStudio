#ifndef Plugin_YSSFileExt_Macro_h
#define Plugin_YSSFileExt_Macro_h
#define YSSFileExtAPI __declspec(dllimport)
#ifdef YSSFileExt_DEVELOPMENT
#undef YSSFileExtAPI
#define YSSFileExtAPI
#endif
#ifdef YSSFileExt_EXPORT
#undef YSSFileExtAPI
#define YSSFileExtAPI __declspec(dllexport)
#endif
#endif // Plugin_YSSFileExt_Macro_h
