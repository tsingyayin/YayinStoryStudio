#pragma once
#include "../../General/Version.h"
#include <QtCore/qstringlist.h>
#include "../../Utility/JsonConfig.h"

namespace YSSCore::Editor {
	class EditorPlugin;
	class EditorPluginManager;
}
namespace YSSCore::__Private__ {
	class EditorPluginPrivate {
		friend YSSCore::Editor::EditorPlugin;
		friend YSSCore::Editor::EditorPluginManager;
	protected:
		YSSCore::Utility::JsonConfig Config;
		YSSCore::General::Version ABIVersion = YSSCore::General::Version(0, 0, 0);
		YSSCore::General::Version PluginVersion = YSSCore::General::Version(0, 0, 0);
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QString PluginFolder;
	};
}