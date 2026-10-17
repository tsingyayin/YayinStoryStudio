#ifndef Plugin_ASERStudio_YSS_ASERFileServer_h
#define Plugin_ASERStudio_YSS_ASERFileServer_h
#include "ASERStudioCompileMacro.h"
#include <Editor/FileServer.h>
#include <Editor/EditorPlugin.h>
#include <Editor/FileEditWidget.h>

namespace ASERStudio::YSS {
	class FileServer_AStoryX :public YSSCore::Editor::FileServer {
	public:
		FileServer_AStoryX(YSSCore::Editor::EditorPlugin* plugin);
		~FileServer_AStoryX();
	};

	class EditorWidget_ASRuleJsonPrivate;
	class EditorWidget_ASRuleJson :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	public:
		EditorWidget_ASRuleJson(QWidget* parent = nullptr);
		virtual ~EditorWidget_ASRuleJson();
	protected:
		virtual bool onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) override;
		virtual bool onClose() override;
		virtual std::expected<QByteArray, bool> onSave(const YSSCore::General::YSSURI& uri) override;
		virtual bool onReload(const QByteArray& data) override;
		virtual bool onCopy() override;
		virtual bool onCut() override;
		virtual bool onPaste() override;
		virtual bool onUndo() override;
		virtual bool onRedo() override;
		virtual bool onSelectAll() override;
	private:
		EditorWidget_ASRuleJsonPrivate* d;
	};

	class FileServer_ASRuleJson :public YSSCore::Editor::FileServer {
	public:
		FileServer_ASRuleJson(YSSCore::Editor::EditorPlugin* plugin);
		~FileServer_ASRuleJson();
		virtual qint64 especiallyFocusFile(const YSSCore::General::YSSURI& uri) override;
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};
}
#endif // Plugin_ASERStudio_YSS_ASERFileServer_h
