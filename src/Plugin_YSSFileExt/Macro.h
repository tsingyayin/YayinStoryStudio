#ifndef YSSFileExt_Macro_h
#define YSSFileExt_Macro_h
#define YSSFileExtAPI __declspec(dllimport)
#ifdef YSSFileExt_DEVELOPMENT
#undef YSSFileExtAPI
#define YSSFileExtAPI
#endif
#ifdef YSSFileExt_EXPORT
#undef YSSFileExtAPI
#define YSSFileExtAPI __declspec(dllexport)
#endif
#endif // YSSFileExt_Macro_h
