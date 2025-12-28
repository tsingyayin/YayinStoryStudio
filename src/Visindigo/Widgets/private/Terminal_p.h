#ifndef Visindigo_Widgets_Terminal_p_h
#define Visindigo_Widgets_Terminal_p_h
#include "../../Macro.h"
#include <QtCore/qobject.h>
class QTextBrowser;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QTextStream;
namespace Visindigo::Widgets {
	class Terminal;
}
namespace Visindigo::__Private__ {
	class TerminalPrivate :public QObject {
		friend class Visindigo::Widgets::Terminal;
		Q_OBJECT;
	protected:
		QTextBrowser* consoleView;
		QLineEdit* inputLine;
		QPushButton* sendButton;
		QGridLayout* layout;
		bool useInput = true;
		bool sendOnEnter = true;
		bool autoScroll = true;
		bool autoListen = true;
		bool enableStyle = true;
		qint32 maxCacheLines = 100;
		QTextStream* stdInStream = nullptr;
		TerminalPrivate();
		~TerminalPrivate();
		void writeToStdIn(const QString& line);
		void onStdInputAvailable();
	};
}
#endif // Visindigo_Widgets_Terminal_p_h