#ifndef Visindigo_Widgets_Terminal_h
#define Visindigo_Widgets_Terminal_h
#include "VICompileMacro.h"
#include <QtWidgets/qframe.h>
namespace Visindigo::__Private__ {
	class TerminalPrivate;
}
namespace Visindigo::Widgets {
	/* IMPORTANT NOTICE :
		This widget will be changed in Visindigo 0.14.0.
	*/
	class VisindigoAPI Terminal :public QFrame {
		Q_OBJECT;
	public:
		Terminal(QWidget* parent = nullptr);
		~Terminal() override;
		void setUseInput(bool enable);
		bool isInputUsed() const;
		void setSendOnEnter(bool enable);
		bool isSendOnEnter() const;
		void setMaxCacheLines(qint32 lineCount);
		qint32 getMaxCacheLines() const;
		void clearConsole();
		void addLine(const QString& line);
	private:
		Visindigo::__Private__::TerminalPrivate* d;
	};
}
#endif // Visindigo_Widgets_Terminal_h