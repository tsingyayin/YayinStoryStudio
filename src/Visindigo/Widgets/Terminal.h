#ifndef Visindigo_Widgets_Terminal_h
#define Visindigo_Widgets_Terminal_h
#include "../Macro.h"
#include <QtWidgets/qframe.h>
namespace Visindigo::__Private__ {
	class TerminalPrivate;
}
namespace Visindigo::Widgets {
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
		void setAutoScroll(bool enable);
		bool isAutoScroll() const;
		void setAutoListen(bool enable);
		bool isAutoListen() const;
		void setEnableStyle(bool enable);
		bool isStyleEnabled() const;
	private:
		Visindigo::__Private__::TerminalPrivate* d;
	};
}
#endif // Visindigo_Widgets_Terminal_h