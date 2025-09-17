#pragma once
#include "EditorPluginModule.h"
// Forward declarations
namespace Visindigo::General {
	class YSSProject;
}
namespace Visindigo::Editor {
	class EditorPlugin;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI EditorProjectHelper :public EditorPluginModule {
		Q_OBJECT;
	public:
		EditorProjectHelper(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin);
		virtual bool isHelperNeeded(Visindigo::General::YSSProject* project) = 0;
		virtual void onProjectLoaded(Visindigo::General::YSSProject* project) = 0;
		virtual void onProjectUnloaded(Visindigo::General::YSSProject* project) = 0;
	};
}