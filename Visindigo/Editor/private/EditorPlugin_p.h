#pragma once
#include "../../General/Version.h"
#include <QtCore/qstringlist.h>
#include "../../Utility/JsonConfig.h"

namespace Visindigo::Editor {
	class EditorPlugin;
	class EditorPluginManager;
}
namespace Visindigo::__Private__ {
	class EditorPluginPrivate {
		friend Visindigo::Editor::EditorPlugin;
		friend Visindigo::Editor::EditorPluginManager;
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