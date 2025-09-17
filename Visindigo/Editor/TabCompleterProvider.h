#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
// Forward declarations
class QTextDocument;
namespace Visindigo::__Private__ {
	class TabCompleterProviderPrivate;
	class TextEditPrivate;
}
namespace Visindigo::Editor {
	class TabCompleterItemPrivate;
	class TextEdit;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI TabCompleterItem {
		friend class TabCompleterItemPrivate;
	public:
		enum ItemType {
			Default = 0,
			Value = 1,
			Const = 2,
			Enum = 3,
			Function = 4,
			Object = 5,
			Operator = 6,
			UserDefined = 1000
		};
	public:
		TabCompleterItem();
		TabCompleterItem(QString text, QString content, QString description = "", ItemType type = ItemType::Default, bool alignment = true);
		~TabCompleterItem();
		VIMoveable(TabCompleterItem);
		VICopyable(TabCompleterItem);
		void setIconPath(const QString& iconPath);
		void setText(const QString& text);
		void setDescription(const QString& description);
		void setContent(const QString& content);
		void setType(ItemType type, bool redirectIcon = true);
		QString getIconPath() const;
		QString getText() const;
		QString getDescription() const;
		QString getContent() const;
		ItemType getType() const;
		bool isAlignment() const;
	private:
		TabCompleterItemPrivate* d;
	};

	class VisindigoAPI TabCompleterProvider :public QObject {
		friend class TextEdit;
		friend class Visindigo::__Private__::TextEditPrivate;
		Q_OBJECT;
	public:
		TabCompleterProvider(QTextDocument* doc);
		virtual ~TabCompleterProvider();
		QTextDocument* getDocument() const;
		virtual QList<TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) = 0;
	protected:
		Visindigo::__Private__::TabCompleterProviderPrivate* d;
	};
}