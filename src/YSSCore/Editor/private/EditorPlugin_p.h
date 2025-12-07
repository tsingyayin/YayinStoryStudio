#pragma once
#include <QtCore/qstringlist.h>
#include <General/Version.h>
#include <Utility/JsonConfig.h>
// Forward declarations
namespace YSSCore::Editor {
	class EditorPlugin;
	class EditorPluginModule;
	class EditorPluginManager;
}
// Main
namespace YSSCore::__Private__ {
	class EditorPluginPrivate {
		friend YSSCore::Editor::EditorPlugin;
		friend YSSCore::Editor::EditorPluginManager;
	protected:
		Visindigo::Utility::JsonConfig Config;
		Visindigo::General::Version ABIVersion = Visindigo::General::Version(0, 0, 0);
		Visindigo::General::Version PluginVersion = Visindigo::General::Version(0, 0, 0);
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QString PluginFolder;
	};
}