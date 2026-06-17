#ifndef YSSCore_Editor_TabCompleterProvider_h
#define YSSCore_Editor_TabCompleterProvider_h
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "YSSCoreCompileMacro.h"
// Forward declarations
class QTextDocument;
namespace YSSCore::__Private__ {
	class TabCompleterProviderPrivate;
	class TextEditPrivate;
}
namespace YSSCore::Editor {
	class TabCompleterItemPrivate;
	class TextEdit;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI TabCompleterItem {
		Q_GADGET;
		friend class TabCompleterItemPrivate;
	public:
		enum ItemType : quint32 {
			Default = 0x0001,
			Value = 0x0002,
			Const = 0x0004,
			Enum = 0x0008,
			Function = 0x0010,
			Object = 0x0020,
			Operator = 0x0040,
			UserDefined = 0x10000,
			AllType = 0xFFFFFFFF
		};
		Q_DECLARE_FLAGS(ItemTypes, ItemType);
		enum CompleterLevel {
			None = -1,
			Few = 0,
			Some = 1,
			Many = 2,
			All = 3
		};
		Q_ENUM(CompleterLevel);
		enum CompleterPriority {
			Low = 0,
			Medium = 1,
			High = 2,
			Top = 3
		};
		Q_ENUM(CompleterPriority);
	public:
		static QString getTypeIconPath(ItemType type);
	public:
		TabCompleterItem();
		TabCompleterItem(QString text, QString content, QString description = "", ItemType type = ItemType::Default, bool alignment = true, 
			CompleterLevel level = CompleterLevel::Some, CompleterPriority priority = CompleterPriority::Low);
		~TabCompleterItem();
		VIMoveable(TabCompleterItem);
		VICopyable(TabCompleterItem);
		void setIconPath(const QString& iconPath);
		void setText(const QString& text);
		void setDescription(const QString& description);
		void setContent(const QString& content);
		void setType(ItemType type, bool redirectIcon = true);
		void setCompleterLevel(CompleterLevel level);
		void setCompleterPriority(CompleterPriority priority);
		QString getIconPath() const;
		QString getText() const;
		QString getDescription() const;
		QString getContent() const;
		ItemType getType() const;
		bool isAlignment() const;
		CompleterLevel getCompleterLevel() const;
		CompleterPriority getCompleterPriority() const;	
	private:
		TabCompleterItemPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(TabCompleterItem::ItemTypes);

	class YSSCoreAPI TabCompleterProvider :public QObject {
		friend class TextEdit;
		friend class YSSCore::__Private__::TextEditPrivate;
		Q_OBJECT;
	public:
		TabCompleterProvider(TextEdit* textEdit);
		virtual ~TabCompleterProvider();
		QTextDocument* getDocument() const;
		virtual QList<TabCompleterItem> onTabComplete(qint32 lineNumber, qint32 column, const QString& content) = 0;
	protected:
		YSSCore::__Private__::TabCompleterProviderPrivate* d;
	};
}
#endif // YSSCore_Editor_TabCompleterProvider_h