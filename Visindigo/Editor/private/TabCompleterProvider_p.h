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
namespace Visindigo::__Private__ {
	class TextEditPrivate;
	class TabCompleterProviderPrivate :public QObject{
		Q_OBJECT;
		friend class Visindigo::Editor::TabCompleterProvider;
		friend class Visindigo::Editor::TabCompleterItem;
	protected:
		Visindigo::Editor::TabCompleterProvider* Q = nullptr;
		QTextDocument* Document = nullptr;
	};

	class TabCompleterWidget :public QFrame {
		Q_OBJECT;
		friend class Visindigo::Editor::TabCompleterItem;
		friend class Visindigo::Editor::TabCompleterProvider;
		friend class Visindigo::Editor::TextEdit;
		friend class Visindigo::__Private__::TextEditPrivate;
	protected:
		QWidget* CentralWidget;
		QScrollArea* ScrollArea;
		QVBoxLayout* Layout;
		QList<QWidget*> Items;
		QMap<Visindigo::Widgets::MultiButton*, Visindigo::Editor::TabCompleterItem> ItemMap;
		QList<Visindigo::Editor::TabCompleterItem> CompleterItems;
		QTextEdit* TextEdit = nullptr;
		Visindigo::Widgets::MultiButtonGroup* ButtonGroup = nullptr;
		TabCompleterWidget(QTextEdit* textEdit, QWidget* parent = nullptr);
		void setCompleterItems(const QList<Visindigo::Editor::TabCompleterItem>& items);
		void selectPrevious();
		void selectNext();
		void doComplete(Visindigo::Widgets::MultiButton* pressed = nullptr);
	};
}