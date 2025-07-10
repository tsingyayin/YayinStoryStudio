#pragma once
#include "../Macro.h"
// Forward declarations
namespace YSSCore::General {
	class YSSProject;
}
namespace YSSCore::Editor{
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	VInterface EditorHelper {
public:
		EditorHelper(EditorPlugin* plugin);
		virtual void onProjectLoaded(YSSCore::General::YSSProject* project) = 0;
		virtual void onProjectUnloaded(YSSCore::General::YSSProject* project) = 0;
	};
}