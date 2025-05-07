#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include "../Macro.h"
#include "../General/Version.h"

namespace YSSCore::Editor {
	class LangServer;
	class DebugServer;
	class FileServer;
	class ProjectTemplateProvider;
	class EditorPluginPrivate;
	class EditorPluginManager;

	class YSSCoreAPI EditorPlugin :public QObject {
		friend class EditorPluginManager;
		friend class EditorPluginPrivate;
		Q_OBJECT;
	public:
		EditorPlugin(YSSCore::General::Version abiVersion = Compiled_YSSABI_Version, QObject* parent = nullptr);
		~EditorPlugin();
		virtual void onPluginEnable() = 0;
		virtual void onPluginDisbale() = 0;
		QString getPluginID() const;
		QString getPluginName() const;
		QStringList getPluginAuthor() const;
		QString getPluginFolder() const;
	protected:
		void setPluginVersion(const YSSCore::General::Version& version);
		void setPluginID(const QString& id);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void setPluginFolder(const QString& folder);
		void registerLangServer(LangServer* server);
		void registerDebugServer(DebugServer* server);
		void registerFileServer(FileServer* server);
		void registerProjectTemplateProvider(ProjectTemplateProvider* provider);
	private:
		EditorPluginPrivate* p;
	};
}

typedef YSSCore::Editor::EditorPlugin* (*__YSSPluginDllMain)(void);