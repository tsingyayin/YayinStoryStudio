#pragma once

#define ASERAPI __declspec(dllimport)
#ifdef ASER_DEVELOPMENT
#undef ASERAPI
#define ASERyAPI
#endif
#ifdef ASER_EXPORT
#undef ASERAPI
#define ASERAPI __declspec(dllexport)
#endif
