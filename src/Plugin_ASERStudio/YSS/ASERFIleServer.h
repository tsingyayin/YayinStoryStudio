#ifndef ASERStudio_YSS_ASERFileServer_h
#define ASERStudio_YSS_ASERFileServer_h
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
		virtual bool onOpen(const QString& path) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& path = "") override;
		virtual bool onReload() override;
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
		virtual qint64 especiallyFocusFile(const QString& filePath) override;
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};
}
#endif // ASERStudio_YSS_ASERFileServer_h