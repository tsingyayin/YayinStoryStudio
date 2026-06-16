#ifndef Visindigo_Widgets_Terminal_p_h
#define Visindigo_Widgets_Terminal_p_h
#include "VICompileMacro.h"
#include "Widgets/Terminal.h"
#include <QtCore/qobject.h>
#include <QtGui/qtextcursor.h>
#include <QtCore/qtypes.h>
#include <QtCore/qtimer.h>
#include "General/TickObject.h"
class QTextBrowser;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QTextStream;
namespace Visindigo::Widgets {
	class Terminal;
}
namespace Visindigo::__Private__ {
	class TerminalPrivate :public QObject, public Visindigo::General::TickObject {
		friend class Visindigo::Widgets::Terminal;
		Q_OBJECT;
	protected:
		Widgets::Terminal* q;
		QTextBrowser* consoleView;
		QLineEdit* inputLine;
		QVBoxLayout* layout;
		qint32 savedCursorLine = -1;
		qint32 savedCursorCol = -1;
		Widgets::Terminal::WorkMode workMode = Widgets::Terminal::VirtualTerminal;
		bool enableInput = true;
		bool autoScroll= true;
		qint32 maxLines = 1000;
		qint32 maxCommandHistory = 1000;
		qint32 historyQueryDelta = 0;
		QString currentHistory = "";
		QTextStream* stdInStream = nullptr;
		QStringList commandHistory;
		QString commandStartWith;
		QString cacheANSILine;
	protected:
		void appendANSICache(const QString& line, bool forceFlush);
		void onANSILineReceived(const QString& line);
		void onComplexControlDetected(const QChar& command, const QString& content);
		qint32 getFirstLineInViewport() const;
		qint32 getLastLineInViewport() const;
		void onColorAndFormatControlDetected(const QString& content);
		void insertPlainText(const QString& text);
		virtual void onFixUpdate(double elapsedTime_ms) override;
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
	};
}
#endif // Visindigo_Widgets_Terminal_p_h