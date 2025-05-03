#include "../LangServer.h"

namespace YSSCore::Editor {
	class LangServerPrivate
	{
		friend class LangServer;
		friend class LangServerManager;
	protected:
		QSyntaxHighlighter* highlighter;
		QString LanguageID;
		QStringList LanguageExt;
		QString pluginFolder = "./resource/server/";
	};

	LangServer::LangServer(QString id, QStringList ext)
	{
		p = new LangServerPrivate();
		p->highlighter = nullptr;
		p->LanguageID = id;
		p->LanguageExt = ext;
	}
	LangServer::~LangServer()
	{
		delete p;
	}
	QSyntaxHighlighter* LangServer::getHighlighter()
	{
		return p->highlighter;
	}

	void LangServer::setHighlighter(QSyntaxHighlighter* highlighter)
	{
		p->highlighter = highlighter;
	}

	QString LangServer::getLangID()
	{
		return p->LanguageID;
	}

	QStringList LangServer::getLangExts()
	{
		return p->LanguageExt;
	}

	QString LangServer::getPluginFolder()
	{
		return p->pluginFolder;
	}

	void LangServer::setPluginFolder(const QString& folder)
	{
		p->pluginFolder = folder;
	}
}