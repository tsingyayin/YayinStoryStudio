#ifndef Plugin_YSSFileExt_Macro_h
#define Plugin_YSSFileExt_Macro_h
#ifdef _MSC_VER
#define YSSFileExtAPI __declspec(dllimport)
#ifdef YSSFileExt_DEVELOPMENT
#undef YSSFileExtAPI
#define YSSFileExtAPI
#endif
#ifdef YSSFileExt_EXPORT
#undef YSSFileExtAPI
#define YSSFileExtAPI __declspec(dllexport)
#endif
#else
// 非 MSVC（Android/Clang/GCC/…）：默认可见性即可，无需 __declspec。
#define YSSFileExtAPI
#endif
#endif // Plugin_YSSFileExt_Macro_h
