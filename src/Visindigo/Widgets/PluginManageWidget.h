#ifndef Visindigo_Widgets_PluginManageWidget_h
#define Visindigo_Widgets_PluginManageWidget_h
#include "../VICompileMacro.h"
#include <QtWidgets/qframe.h>
namespace Visindigo::Widgets {
	class PluginManageWidgetPrivate;
	class VisindigoAPI PluginManageWidget :public QFrame {
		Q_OBJECT;
	public:
		PluginManageWidget(QWidget* parent = nullptr);
		~PluginManageWidget();
		void refreshPluginList();
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		PluginManageWidgetPrivate* d;
	};
}
#endif // Visindigo_Widgets_PluginManageWidget_h