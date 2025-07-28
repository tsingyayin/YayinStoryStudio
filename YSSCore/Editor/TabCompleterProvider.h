#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
class QTextDocument;
namespace YSSCore::__Private__ {
	class TabCompleterProviderPrivate;
	class TextEditPrivate;
}
namespace YSSCore::Editor {
	class TabCompleterItemPrivate;
	class TextEdit;
}

namespace YSSCore::Editor {
	class YSSCoreAPI TabCompleterItem {
		friend class TabCompleterItemPrivate;
	public:
		TabCompleterItem();
		TabCompleterItem(QString iconPath, QString text, QString description, QString content, bool alignment = true);
		~TabCompleterItem();
		VIMoveable(TabCompleterItem);
		VICopyable(TabCompleterItem);
		QString getIconPath() const;
		QString getText() const;
		QString getDescription() const;
		QString getContent() const;
		bool isAlignment() const;
	private:
		TabCompleterItemPrivate* d;
	};

	class YSSCoreAPI TabCompleterProvider :public QObject{
		friend class TextEdit;
		friend class YSSCore::__Private__::TextEditPrivate;
		Q_OBJECT;
	public:
		TabCompleterProvider(QTextDocument* doc);
		virtual ~TabCompleterProvider();
		QTextDocument* getDocument() const;
		virtual QList<TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) = 0;
	protected:
		YSSCore::__Private__::TabCompleterProviderPrivate* d;
	};
}