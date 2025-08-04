#pragma once
#include "EditorPluginModule.h"
// Forward declarations
namespace YSSCore::General {
	class YSSProject;
}
namespace YSSCore::Editor{
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI EditorProjectHelper :public EditorPluginModule {
		Q_OBJECT;
public:
		EditorProjectHelper(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin);
		virtual bool isHelperNeeded(YSSCore::General::YSSProject* project)  = 0;
		virtual void onProjectLoaded(YSSCore::General::YSSProject* project) = 0;
		virtual void onProjectUnloaded(YSSCore::General::YSSProject* project) = 0;
	};
}