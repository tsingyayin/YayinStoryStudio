#include <QtCore/qstring.h>
#include <QtGui/qsyntaxhighlighter.h>
#include "../LangServer.h"
#include <General/Plugin.h>
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class LangServerPrivate
	{
		friend class LangServer;
		friend class LangServerManager;
	protected:
		QString LanguageID;
		QStringList LanguageExt;
	};

	LangServer::LangServer(const QString& name, const QString& id, EditorPlugin* plugin, const QString& lang_id, QStringList ext) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_LangServer, name)
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