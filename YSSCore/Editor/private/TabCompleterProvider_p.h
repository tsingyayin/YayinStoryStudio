#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtWidgets/qframe.h>
#include "../TabCompleterProvider.h"
// Forward declarations
class QTextDocument;
class QTextCursor;
class QWidget;
class QScrollArea;
class QHBoxLayout;
// Main Implementation
namespace YSSCore::__Private__ {
	class TextEditPrivate;
	class TabCompleterProviderPrivate :public QObject{
		Q_OBJECT;
		friend class YSSCore::Editor::TabCompleterProvider;
		friend class YSSCore::Editor::TabCompleterItem;
	protected:
		YSSCore::Editor::TabCompleterProvider* Q = nullptr;
		QTextDocument* Document = nullptr;
	};

	class TabCompleterWidget :public QFrame {
		Q_OBJECT;
		friend class YSSCore::Editor::TabCompleterItem;
		friend class YSSCore::Editor::TabCompleterProvider;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::__Private__::TextEditPrivate;
	protected:
		QWidget* CentralWidget;
		QScrollArea* ScrollArea;
		QHBoxLayout* Layout;
		QList<QWidget*> Items;
		QList<YSSCore::Editor::TabCompleterItem> CompleterItems;
		TabCompleterWidget(QWidget* parent = nullptr);
		void setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items);
	};
}