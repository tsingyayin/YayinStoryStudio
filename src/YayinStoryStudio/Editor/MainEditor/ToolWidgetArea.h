#ifndef YSS_Editor_MainWin_ToolWidgetArea_h
#define YSS_Editor_MainWin_ToolWidgetArea_h
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include <Editor/ToolWidgetManager.h>
#include <Widgets/BorderFrame.h>
namespace YSS::Editor {
	class ToolWidgetAreaPrivate;
	class ToolWidgetArea :public Visindigo::Widgets::BorderFrame {
		Q_OBJECT;
	signals:
		void closed(const QString& widgetID);
		void currentWidgetChanged(const QString& widgetID);
	public:
		ToolWidgetArea(QWidget* parent = nullptr);
		virtual ~ToolWidgetArea();
		void addWidget(const QString& widgetID);
		void closeAll();
		void closeWidget(const QString& widgetID);
		bool containsWidget(const QString& widgetID) const;
		void setCurrentWidget(const QString& widgetID);
		QWidget* getCurrentWidget() const;
	private:
		ToolWidgetAreaPrivate* d;
	};
}
#endif //YSS_Editor_MainWin_ToolWidgetArea_h
