#pragma once
#include <QtGui>
#include "../Macro.h"
#include <QSyntaxHighlighter>

namespace YSSCore::Editor {
	class LangServerPrivate;
	class YSSCoreAPI LangServer{
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(QString id, QStringList ext);
		~LangServer();
		virtual void serverEnable() = 0;
		virtual void serverDisable() = 0;
		virtual QSyntaxHighlighter* createHighlighter() = 0;
		QString getLangID();
		QStringList getLangExts();
		QString getPluginFolder();
	protected:
		LangServerPrivate* p;
		void setPluginFolder(const QString& folder);
	};
}

typedef YSSCore::Editor::LangServer* (*__YSSLangServerDllMain)(void);
