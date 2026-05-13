#ifndef YSS_Editor_MainWin_ToolWidgetArea_h
#define YSS_Editor_MainWin_ToolWidgetArea_h
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include <Editor/ToolWidgetManager.h>

namespace YSS::Editor {
	class ToolWidgetAreaPrivate;
	class ToolWidgetArea :public QFrame {
		Q_OBJECT;
	signals:
		void closed(const QString& widgetID);
	public:
		ToolWidgetArea(QWidget* parent = nullptr) {};
		virtual ~ToolWidgetArea() {};
		void addToolWidget(const QString& widgetID);
		void closeToolWidget(const QString& widgetID);
		bool containsToolWidget(const QString& widgetID) const;
		void setCurrentToolWidget(const QString& widgetID);
	private:
		ToolWidgetAreaPrivate* d;
	};
}
#endif //YSS_Editor_MainWin_ToolWidgetArea_h
