#include "../LangServer.h"
#include <QtCore/qstring.h>
#include <QtGui/qsyntaxhighlighter.h>

namespace YSSCore::Editor {
	class LangServerPrivate
	{
		friend class LangServer;
		friend class LangServerManager;
	protected:
		QString LanguageID;
		QStringList LanguageExt;
		EditorPlugin* Plugin;
	};

	LangServer::LangServer(EditorPlugin* plugin, QString id, QStringList ext)
	{
		d = new LangServerPrivate();
		d->Plugin = plugin;
		d->LanguageID = id;
		d->LanguageExt = ext;
	}
	LangServer::~LangServer()
	{
		delete d;
	}
	QString LangServer::getLangID()
	{
		return d->LanguageID;
	}

	QStringList LangServer::getLangExts()
	{
		return d->LanguageExt;
	}

	EditorPlugin* LangServer::getPlugin() {
		return d->Plugin;
	}
}