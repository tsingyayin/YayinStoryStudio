#pragma once
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
#include <Widgets/PluginManageWidget.h>
#include <QtWidgets/qboxlayout.h>
namespace YSS::Editor {
	class PreferenceVFServer :public YSSCore::Editor::FileServer {
	public:
		PreferenceVFServer(YSSCore::Editor::EditorPlugin* plugin);
		~PreferenceVFServer();
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class PreferenceEditWidget :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	private:
		Visindigo::Widgets::PluginManageWidget* pluginManageWidget;
		QVBoxLayout* layout;
	public:
		PreferenceEditWidget(QWidget* parent = nullptr);
		virtual ~PreferenceEditWidget();
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) override;
		virtual bool onVirtualClose() override;
		virtual bool onVirtualSave() override;
	};
}