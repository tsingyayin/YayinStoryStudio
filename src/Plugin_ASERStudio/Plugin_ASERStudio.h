#pragma once
#include <Editor/EditorPlugin.h>
#include <General/Translator.h>
namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASERDebugIO;
}
namespace ASERStudio {
	class ASERStudioTranslator :public Visindigo::General::Translator {
	public:
		ASERStudioTranslator(Visindigo::General::Plugin* parent);
	};

	class MainPrivate;
	class Main : public YSSCore::Editor::EditorPlugin {
		Q_OBJECT;
	public:
		Main();
		static Main* getInstance();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onProjectOpen(YSSCore::General::YSSProject* project) override;
		virtual void onProjectClose(YSSCore::General::YSSProject* project) override;
		virtual void onTest() override;
		void onConfigLoaded();
		QWidget* getConfigWidget() override;
		ASEREnv::ASERDebugIO* getASERDebugIO() const;
		ASEREnv::ASERProgram* getASERProgram() const;
	private:
		MainPrivate* d;
	};
}