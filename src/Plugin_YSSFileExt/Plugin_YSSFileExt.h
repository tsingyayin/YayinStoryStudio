#pragma once
#include <Editor/EditorPlugin.h>

class Plugin_YSSFileExt : public YSSCore::Editor::EditorPlugin {
	Q_OBJECT;
public:
	Plugin_YSSFileExt();
	virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) override { return nullptr; };
	virtual void onPluginEnable() override;
	virtual void onPluginDisbale() override;
};