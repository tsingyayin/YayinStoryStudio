#ifndef Plugin_ASERStudio_YSS_ASERProjectTemplate_h
#define Plugin_ASERStudio_YSS_ASERProjectTemplate_h
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

	class ASERAPI ProjectTemplate_AStoryX :public YSSCore::Editor::ProjectTemplateProvider {
	public:
		ProjectTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~ProjectTemplate_AStoryX();
		virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
	};

	class ProjectTemplateInitWidget_AStoryX_3_6_7Private;
	class ASERAPI ProjectTemplateInitWidget_AStoryX_3_6_7 :public YSSCore::Editor::ProjectTemplateInitWidget {
		Q_OBJECT;
	public:
		ProjectTemplateInitWidget_AStoryX_3_6_7(QWidget* parent = nullptr);
		virtual ~ProjectTemplateInitWidget_AStoryX_3_6_7();
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onLineEditTextChanged(const QString& node, const QString& str);
		void onCreateButtonClicked();
		void initProjectV3(YSSCore::General::YSSProject* project);
	private:
		void refreshWhereLabel();
	private:
		ProjectTemplateInitWidget_AStoryX_3_6_7Private* d;
	};

	class ASERAPI ProjectTemplate_AStoryX_3_6_7 :public YSSCore::Editor::ProjectTemplateProvider {
	public:
		ProjectTemplate_AStoryX_3_6_7(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~ProjectTemplate_AStoryX_3_6_7();
		virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
	};
}
#endif // Plugin_ASERStudio_YSS_ASERProjectTemplate_h
