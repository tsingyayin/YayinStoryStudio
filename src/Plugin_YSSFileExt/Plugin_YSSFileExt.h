#ifndef Plugin_YSSFileExt_Plugin_YSSFileExt_h
#define Plugin_YSSFileExt_Plugin_YSSFileExt_h
#include <Editor/EditorPlugin.h>
#include <General/Translator.h>

namespace YSSFileExt {
	class YSSFileExtTranslator :public Visindigo::General::Translator {
	public:
		YSSFileExtTranslator(Visindigo::General::Plugin* parent);
	};
}

class Plugin_YSSFileExt : public YSSCore::Editor::EditorPlugin {
	Q_OBJECT;
public:
	Plugin_YSSFileExt();
	virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) override { return nullptr; };
	virtual void onPluginEnable() override;
	virtual void onPluginDisable() override;
};
#endif // Plugin_YSSFileExt_Plugin_YSSFileExt_h
