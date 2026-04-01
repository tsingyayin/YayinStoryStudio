#ifndef ASERStudio_YSS_ASERProjectTemplate_h
#define ASERStudio_YSS_ASERProjectTemplate_h
#include "ASERStudioCompileMacro.h"
#include <Editor/ProjectTemplateProvider.h>
#include <General/YSSProject.h>
namespace ASERStudio::YSS {
	class ProjectTemplateInitWidget_AStoryXPrivate;
	class ASERAPI ProjectTemplateInitWidget_AStoryX :public YSSCore::Editor::ProjectTemplateInitWidget {
		Q_OBJECT;
	public:
		ProjectTemplateInitWidget_AStoryX(QWidget* parent = nullptr);
		virtual ~ProjectTemplateInitWidget_AStoryX();
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onLineEditTextChanged(const QString& node, const QString& str);
		void onCreateButtonClicked();
		void initProjectV3(YSSCore::General::YSSProject* project);
	private:
		void refreshWhereLabel();
	private:
		ProjectTemplateInitWidget_AStoryXPrivate* d;
	};
	class ASERAPI ProjectTemplate_AStoryX : public YSSCore::Editor::ProjectTemplateProvider {
	public:
		ProjectTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~ProjectTemplate_AStoryX();
		virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
	};
}
#endif // ASERStudio_YSS_ASERProjectTemplate_h
