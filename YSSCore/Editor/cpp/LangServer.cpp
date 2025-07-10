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
	};

	LangServer::LangServer(const QString& name, const QString& id, EditorPlugin* plugin, const QString& lang_id , QStringList ext):
		EditorPluginModule(name, id, plugin)
	{
		d = new LangServerPrivate();
		d->LanguageID = lang_id;
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