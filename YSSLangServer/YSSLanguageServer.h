#pragma once
#include "macro.h"
#include <QtGui>
class YSSLanguageServerPrivate;

class YSSAPI YSSLanguageServer {
	friend class YSSLanguageServerPrivate;
public:
	YSSLanguageServer(QString id, QStringList ext);
	QSyntaxHighlighter* getHighlighter();
	void setHighlighter(QSyntaxHighlighter* highlighter);
	QString getLanguageID();
	QStringList getLanguageExt();
private:
	YSSLanguageServerPrivate* p;
};

typedef YSSLanguageServer* (DLLMain)();