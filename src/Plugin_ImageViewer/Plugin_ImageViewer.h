#ifndef Plugin_ImageViewer_Plugin_ImageViewer_h
#define  Plugin_ImageViewer_Plugin_ImageViewer_h
#include <Editor/EditorPlugin.h>
#include <General/Translator.h>
namespace YSS::ImageViewer {
	class Translator :public Visindigo::General::Translator {
	public:
		Translator(Visindigo::General::Plugin* parent);
	};

	class Main : public YSSCore::Editor::EditorPlugin {
		Q_OBJECT;
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onProjectOpen(YSSCore::General::YSSProject* project) override;
		virtual void onProjectClose(YSSCore::General::YSSProject* project) override;
		virtual void onTest() override;
	};
}
#endif // Plugin_ImageViewer_Plugin_ImageViewer_h
