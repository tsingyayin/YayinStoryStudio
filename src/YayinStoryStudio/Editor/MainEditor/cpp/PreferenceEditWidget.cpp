#include "Editor/MainEditor/PreferenceEditWidget.h"

namespace YSS::Editor {
	PreferenceVFServer::PreferenceVFServer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("Preference Virtual File Server", "YSS.Editor.PreferencesVFServer", plugin) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts({ "YSS.MainEditor.Preferences" });
		setAsVitrualFileServer(true);
	}
	PreferenceVFServer::~PreferenceVFServer() {}
	
	YSSCore::Editor::FileEditWidget* PreferenceVFServer::onCreateFileEditWidget() {
		return new PreferenceEditWidget();
	}

	PreferenceEditWidget::PreferenceEditWidget(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		pluginManageWidget = new Visindigo::Widgets::PluginManageWidget(this);
		layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(pluginManageWidget);
	}

	PreferenceEditWidget::~PreferenceEditWidget() {

	}

	bool PreferenceEditWidget::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.Preferences") {
			return true;
		}
		return false;
	}

	bool PreferenceEditWidget::onVirtualClose() {
		return true;
	}

	bool PreferenceEditWidget::onVirtualSave() {
		return true;
	}
}