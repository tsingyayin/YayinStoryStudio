#ifndef YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
#define YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
#include <QtWidgets/qwidget.h>
namespace YSS::Editor {
	class ColorThemeSettingsWidgetPrivate;
	class ColorThemeSettingsWidget :public QWidget {
		Q_OBJECT;
	private:
		ColorThemeSettingsWidgetPrivate* d;
	public:
		ColorThemeSettingsWidget(QWidget* parent = nullptr);
		virtual ~ColorThemeSettingsWidget();
	public:
		virtual void resizeEvent(QResizeEvent* event) override;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
