#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtWidgets/qframe.h>
#include "../TabCompleterProvider.h"
#include <Widgets/BorderFrame.h>
#include <Widgets/ThemeManager.h>
// Forward declarations
class QTextDocument;
class QTextCursor;
class QScrollBar;
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

	class TabCompleterWidget :public Visindigo::Widgets::BorderFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
		friend class YSSCore::Editor::TabCompleterItem;
		friend class YSSCore::Editor::TabCompleterProvider;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::__Private__::TextEditPrivate;
	protected:
		QTextEdit* Target;
		QScrollBar* ScrollBar;
		QList<YSSCore::Editor::TabCompleterItem> Items;
		QList<Visindigo::Widgets::MultiButton*> Buttons;
		QList<qint32> ButtonCycleIndexes;
		qint32 currentSelectedIndex = -1;
		qint32 buttonCacheSize = 18;
		qint32 buttonHeight = 30;
		qint32 maxAllowedHeight = 0;
		TabCompleterWidget(QTextEdit* textEdit);
		void setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items);
		void selectPrevious();
		void selectNext();
		void doComplete(Visindigo::Widgets::MultiButton* pressed = nullptr);
		void scrollBy(qint32 y);
		void onScrollValueChanged(qint32 value);
		void adjustHeight(qint32 height);
		qint32 getMaxAllowedHeight() const;
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void onThemeChanged() override;
	};
}