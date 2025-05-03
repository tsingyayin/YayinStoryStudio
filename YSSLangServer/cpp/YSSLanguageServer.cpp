#include "../YSSLanguageServer.h"

class YSSLanguageServerPrivate
{
	friend class YSSLanguageServer;
protected:
	QSyntaxHighlighter* highlighter;
	QString LanguageID;
	QStringList LanguageExt;
};

YSSLanguageServer::YSSLanguageServer(QString id, QStringList ext)
{
	p = new YSSLanguageServerPrivate();
	p->highlighter = nullptr;
	p->LanguageID = id;
	p->LanguageExt = ext;
}

QSyntaxHighlighter* YSSLanguageServer::getHighlighter()
{
	return p->highlighter;
}

void YSSLanguageServer::setHighlighter(QSyntaxHighlighter* highlighter)
{
	p->highlighter = highlighter;
}

QString YSSLanguageServer::getLanguageID()
{
	return p->LanguageID;
}

QStringList YSSLanguageServer::getLanguageExt()
{
	return p->LanguageExt;
}