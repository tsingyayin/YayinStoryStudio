#ifndef YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
#define YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
#include <Widgets/PluginManageWidget.h>
#include <QtWidgets/qboxlayout.h>
namespace YSS::Editor {
	class ColorThemeSettingsVFServer :public YSSCore::Editor::FileServer {
	public:
		ColorThemeSettingsVFServer(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class ColorThemeSettingsEditWidgetPrivate;
	class ColorThemeSettingsEditWidget :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	private:
		ColorThemeSettingsEditWidgetPrivate* d;
	public:
		ColorThemeSettingsEditWidget(QWidget* parent = nullptr);
		virtual ~ColorThemeSettingsEditWidget();
	public:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) override;
		virtual bool onVirtualClose() override;
		virtual bool onVirtualSave() override;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_ColorThemeSettings_h
