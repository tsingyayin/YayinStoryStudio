#pragma once

#define YSSLS_AStory __declspec(dllimport)
#ifdef YSSLS_AStory_DEVELOPMENT
#undef YSSLS_AStory
#define YSSLS_AStory
#endif
#ifdef YSSLS_AStory_EXPORT
#undef YSSLS_AStory
#define YSSLS_AStory __declspec(dllexport)
#endif
