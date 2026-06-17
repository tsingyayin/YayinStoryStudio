#ifndef Visindigo_Widgets_Terminal_h
#define Visindigo_Widgets_Terminal_h
#include "VICompileMacro.h"
#include <QtWidgets/qframe.h>
#include <QtCore/qprocess.h>
#include <QtCore/qstringlist.h>
namespace Visindigo::__Private__ {
	class TerminalPrivate;
}
namespace Visindigo::Widgets {
	class VisindigoAPI Terminal :public QFrame {
		Q_OBJECT;
	public:
		enum WorkMode {
			PureText, // display anything as plain text, without any format or control character processing.
			VirtualTerminal // support ANSI escape codes for text formatting and cursor control, default mode.
		};
	signals:
		void inputPrepared(const QString& line);
		void externalProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
		void stdoutReceived(const QString& text);
		void stderrReceived(const QString& text);
	public:
		static Terminal* createPowerShell();
	public:
		Terminal(QWidget* parent = nullptr);
		~Terminal() override;
		void setInputEnable(bool enable);
		bool isInputEnabled() const;
		QStringList getCommandHistory() const;
		void setMaxCommandHistory(qint32 count);
		qint32 getMaxCommandHistory() const;
		void setAutoScroll(bool enable);
		bool isAutoScroll() const;
		void setMaxLines(qint32 count);
		qint32 getMaxLines() const;
		void clearConsole();
		void input(const QString& line);
		void addLine(const QString& line, bool forceFlush = false);
		void setWorkMode(WorkMode mode);
		WorkMode getWorkMode() const;
		void setExternalProcessUTF8(bool utf8);
		bool isExternalProcessUTF8() const;
		void launchExternalProcess(const QString& systemCommand, const QStringList& arguments, const QString& workingDirectory = QString());
		bool isExternalProcessRunning() const;
		void terminateExternalProcess(bool waiting = false);
		QProcess* getExternalProcess() const;
	private:
		Visindigo::__Private__::TerminalPrivate* d;
	};
}
#endif // Visindigo_Widgets_Terminal_h