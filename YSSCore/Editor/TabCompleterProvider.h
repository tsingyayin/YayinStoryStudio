#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
class QTextDocument;
namespace YSSCore::__Private__ {
	class TabCompleterProviderPrivate;
}
namespace YSSCore::Editor {
	class TabCompleterItemPrivate;
	class TabCompleterItem {
		friend class TabCompleterItemPrivate;
	public:
		TabCompleterItem(QString iconPath, QString text, QString description, QString content, bool alignment = true);
		~TabCompleterItem();
		VIMoveable(TabCompleterItem);
		VICopyable(TabCompleterItem);
	private:
		TabCompleterItemPrivate* d;
	};

	class TabCompleterProvider :public QObject{
	public:
		TabCompleterProvider(QTextDocument* parent);
		void setDocument(QTextDocument* doc);
		QTextDocument* getDocument() const;
		virtual void onDocumentChanged(QTextDocument* doc) = 0;
		virtual QList<TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) = 0;
	private:
		YSSCore::__Private__::TabCompleterProviderPrivate* d;
	};
}