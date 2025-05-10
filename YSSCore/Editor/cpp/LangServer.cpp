#include "../LangServer.h"
#include "../ThemeManager.h"
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
	};

	LangServer::LangServer(QString id, QStringList ext)
	{
		d = new LangServerPrivate();
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
}