#ifndef YSSFileExt_Plugin_YSSFileExt_h
#define YSSFileExt_Plugin_YSSFileExt_h
#include <Editor/EditorPlugin.h>

class Plugin_YSSFileExt : public YSSCore::Editor::EditorPlugin {
	Q_OBJECT;
public:
	Plugin_YSSFileExt();
	virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) override { return nullptr; };
	virtual void onPluginEnable() override;
	virtual void onPluginDisable() override;
};
#endif // YSSFileExt_Plugin_YSSFileExt_h
