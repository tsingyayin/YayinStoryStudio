#pragma once
#include <Editor/EditorPlugin.h>
#include <General/Translator.h>
namespace ASERStudio {
	class ASERStudioTranslator :public Visindigo::General::Translator {
	public:
		ASERStudioTranslator(Visindigo::General::Plugin* parent);
	};

	class Main : public YSSCore::Editor::EditorPlugin {
		Q_OBJECT;
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onTest() override;
	};
}