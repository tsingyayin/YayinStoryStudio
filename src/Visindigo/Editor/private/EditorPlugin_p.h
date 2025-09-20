#pragma once
#include <QtCore/qstringlist.h>
#include "../../General/Version.h"
#include "../../Utility/JsonConfig.h"
// Forward declarations
namespace Visindigo::Editor {
	class EditorPlugin;
	class EditorPluginModule;
	class EditorPluginManager;
}
// Main
namespace Visindigo::__Private__ {
	class EditorPluginPrivate {
		friend Visindigo::Editor::EditorPlugin;
		friend Visindigo::Editor::EditorPluginManager;
	protected:
		QList<Visindigo::Editor::EditorPluginModule*> Modules;
		Visindigo::Utility::JsonConfig Config;
		Visindigo::General::Version ABIVersion = Visindigo::General::Version(0, 0, 0);
		Visindigo::General::Version PluginVersion = Visindigo::General::Version(0, 0, 0);
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QString PluginFolder;
	};
}