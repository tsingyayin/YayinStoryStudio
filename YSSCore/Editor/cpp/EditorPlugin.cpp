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
		p = new EditorPluginPrivate();
		p->ABIVersion = abiVersion;
	}
	EditorPlugin::~EditorPlugin() {
		delete p;
	}
	QString EditorPlugin::getPluginID() const {
		return p->PluginID;
	}
	QString EditorPlugin::getPluginName() const {
		return p->PluginName;
	}
	QStringList EditorPlugin::getPluginAuthor() const {
		return p->PluginAuthor;
	}
	QString EditorPlugin::getPluginFolder() const {
		return p->PluginFolder;
	}
	void EditorPlugin::setPluginID(const QString& id) {
		p->PluginID = id;
	}
	void EditorPlugin::setPluginName(const QString& name) {
		p->PluginName = name;
	}
	void EditorPlugin::setPluginAuthor(const QStringList& author) {
		p->PluginAuthor = author;
	}
	void EditorPlugin::setPluginFolder(const QString& folder) {
		p->PluginFolder = folder;
	}
	void EditorPlugin::setPluginVersion(const YSSCore::General::Version& version) {
		p->PluginVersion = version;
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