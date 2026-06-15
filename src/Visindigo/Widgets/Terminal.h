#ifndef Visindigo_Widgets_Terminal_h
#define Visindigo_Widgets_Terminal_h
#include "VICompileMacro.h"
#include <QtWidgets/qframe.h>
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
	public:
		Terminal(QWidget* parent = nullptr);
		~Terminal() override;
		void setInputEnable(bool enable);
		bool isInputEnabled() const;
		QStringList getInputHistory() const;
		void setMaxInputHistory(qint32 count);
		qint32 getMaxInputHistory() const;
		void setAutoScroll(bool enable);
		bool isAutoScroll() const;
		void setMaxLines(qint32 count);
		qint32 getMaxLines() const;
		void clearConsole();
		void addLine(const QString& line);
		void setWorkMode(WorkMode mode);
		void setRefreshDelay(qint32 ms);
		qint32 getRefreshDelay();
		WorkMode getWorkMode() const;
	private:
		Visindigo::__Private__::TerminalPrivate* d;
	};
}
#endif // Visindigo_Widgets_Terminal_h