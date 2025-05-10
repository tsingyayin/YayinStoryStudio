#include <QtCore/qstring.h>
#include "../EditorPlugin.h"
#include "../LangServerManager.h"
#include "../FileServerManager.h"
namespace YSSCore::Editor {
	class EditorPluginPrivate {
		friend class EditorPlugin;
	protected:
		YSSCore::General::Version ABIVersion = YSSCore::General::Version(0, 0, 0);
		YSSCore::General::Version PluginVersion = YSSCore::General::Version(0, 0, 0);
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QString PluginFolder;
	};
	EditorPlugin::EditorPlugin(YSSCore::General::Version abiVersion, QObject* parent) : QObject(parent) {
		d = new EditorPluginPrivate();
		d->ABIVersion = abiVersion;
	}
	EditorPlugin::~EditorPlugin() {
		delete d;
	}
	QString EditorPlugin::getPluginID() const {
		return d->PluginID;
	}
	QString EditorPlugin::getPluginName() const {
		return d->PluginName;
	}
	QStringList EditorPlugin::getPluginAuthor() const {
		return d->PluginAuthor;
	}
	QString EditorPlugin::getPluginFolder() const {
		return d->PluginFolder;
	}
	void EditorPlugin::setPluginID(const QString& id) {
		d->PluginID = id;
	}
	void EditorPlugin::setPluginName(const QString& name) {
		d->PluginName = name;
	}
	void EditorPlugin::setPluginAuthor(const QStringList& author) {
		d->PluginAuthor = author;
	}
	void EditorPlugin::setPluginFolder(const QString& folder) {
		d->PluginFolder = folder;
	}
	void EditorPlugin::setPluginVersion(const YSSCore::General::Version& version) {
		d->PluginVersion = version;
	}
	void EditorPlugin::registerLangServer(LangServer* server) {
		LangServerManager::getInstance()->addLangServer(server);
	}
	void EditorPlugin::registerDebugServer(DebugServer* server) {
		//TODO;
	}
	void EditorPlugin::registerFileServer(FileServer* server) {
		FileServerManager::getInstance()->registerFileServer(server);
	}
	void EditorPlugin::registerProjectTemplateProvider(ProjectTemplateProvider* provider) {
		//TODO;
	}
}