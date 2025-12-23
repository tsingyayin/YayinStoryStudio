#ifndef Visindigo_Widgets_VirtualConsole_p_h
#define Visindigo_Widgets_VirtualConsole_p_h
#include "../../Macro.h"
#include <QtCore/qobject.h>
class QTextBrowser;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QTextStream;
namespace Visindigo::Widgets {
	class VirtualConsole;
}
namespace Visindigo::__Private__ {
	class VirtualConsolePrivate :public QObject {
		friend class Visindigo::Widgets::VirtualConsole;
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
		QTextStream* stdOutStream = nullptr;
		QTextStream* stdErrStream = nullptr;
		VirtualConsolePrivate();
		~VirtualConsolePrivate();
		void writeToStdIn(const QString& line);
		void onStdInputAvailable();
		void onStdOutputAvailable();
		void onStdErrorAvailable();
	};
}
#endif // Visindigo_Widgets_VirtualConsole_p_h