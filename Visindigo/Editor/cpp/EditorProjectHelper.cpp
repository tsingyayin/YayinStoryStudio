#include "../EditorProjectHelper.h"

namespace Visindigo::Editor {
	EditorProjectHelper::EditorProjectHelper(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin)
		:EditorPluginModule(moduleName, moduleID, plugin) {
		
	}
}