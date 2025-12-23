#ifndef Visindigo_Widgets_VirtualConsole_h
#define Visindigo_Widgets_VirtualConsole_h
#include "../Macro.h"
#include <QtWidgets/qframe.h>
namespace Visindigo::__Private__ {
	class VirtualConsolePrivate;
}
namespace Visindigo::Widgets {
	class VisindigoAPI VirtualConsole :public QFrame {
		Q_OBJECT;
	public:
		VirtualConsole(QWidget* parent = nullptr);
		~VirtualConsole() override;
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
		Visindigo::__Private__::VirtualConsolePrivate* d;
	};
}
#endif // Visindigo_Widgets_VirtualConsole_h