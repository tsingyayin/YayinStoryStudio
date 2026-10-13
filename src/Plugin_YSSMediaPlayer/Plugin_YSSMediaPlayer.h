#ifndef Plugin_YSSMediaPlayer_Plugin_YSSMediaPlayer_h
#define Plugin_YSSMediaPlayer_Plugin_YSSMediaPlayer_h
#include <Editor/EditorPlugin.h>
#include <General/Translator.h>

namespace YSS::MediaPlayer {
	class MediaPlayerTranslator :public Visindigo::General::Translator {
	public:
		MediaPlayerTranslator(Visindigo::General::Plugin* parent);
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
#endif // Plugin_YSSMediaPlayer_Plugin_YSSMediaPlayer_h
