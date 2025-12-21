#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtWidgets/qframe.h>
#include "../TabCompleterProvider.h"
// Forward declarations
class QTextDocument;
class QTextCursor;
class QWidget;
class QTextEdit;
class QScrollArea;
class QVBoxLayout;
namespace Visindigo::Widgets {
	class MultiButton;
	class MultiButtonGroup;
}
// Main Implementation
namespace YSSCore::__Private__ {
	class TextEditPrivate;
	class TabCompleterProviderPrivate :public QObject {
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
		QVBoxLayout* Layout;
		QList<QWidget*> Items;
		QMap<Visindigo::Widgets::MultiButton*, YSSCore::Editor::TabCompleterItem> ItemMap;
		QList<YSSCore::Editor::TabCompleterItem> CompleterItems;
		QTextEdit* TextEdit = nullptr;
		Visindigo::Widgets::MultiButtonGroup* ButtonGroup = nullptr;
		TabCompleterWidget(QTextEdit* textEdit);
		void setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items);
		void selectPrevious();
		void selectNext();
		void doComplete(Visindigo::Widgets::MultiButton* pressed = nullptr);
	};
}