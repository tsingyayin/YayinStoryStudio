#include "../LangServer.h"
#include "../ThemeManager.h"
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
		p = new LangServerPrivate();
		p->LanguageID = id;
		p->LanguageExt = ext;
	}
	LangServer::~LangServer()
	{
		delete p;
	}
	QString LangServer::getLangID()
	{
		return p->LanguageID;
	}

	QStringList LangServer::getLangExts()
	{
		return p->LanguageExt;
	}
}