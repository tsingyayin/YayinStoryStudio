#pragma once

#define AStoryAPI __declspec(dllimport)
#ifdef AStory_DEVELOPMENT
#undef AStoryAPI
#define AStoryAPI
#endif
#ifdef AStory_EXPORT
#undef AStoryAPI
#define AStoryAPI __declspec(dllexport)
#endif
