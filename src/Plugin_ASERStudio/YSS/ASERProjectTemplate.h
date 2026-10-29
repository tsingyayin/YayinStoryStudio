#ifndef Plugin_ASERStudio_YSS_ASERProjectTemplate_h
#define Plugin_ASERStudio_YSS_ASERProjectTemplate_h
#include "ASERStudioCompileMacro.h"
#include <Editor/ProjectTemplateProvider.h>
namespace ASERStudio::YSS {
	class ProjectTemplateInitWidget_AStoryXPrivate;
	class ASERAPI ProjectTemplateInitWidget_AStoryX :public YSSCore::Editor::ProjectTemplateInitWidget {
		Q_OBJECT;
	public:
		ProjectTemplateInitWidget_AStoryX(const YSSCore::General::YSSURI& targetUri, QWidget* parent = nullptr);
		virtual ~ProjectTemplateInitWidget_AStoryX();
	public slots:
		void onCreateButtonClicked();
	private:
		bool prepareProjectV3();
		void refreshWhereLabel();
	private:
		ProjectTemplateInitWidget_AStoryXPrivate* d;
	};

	class ASERAPI ProjectTemplate_AStoryX :public YSSCore::Editor::ProjectTemplateProvider {
	public:
		ProjectTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~ProjectTemplate_AStoryX();
		virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget(
			const YSSCore::General::YSSURI& targetUri) override;
	};

	class ProjectTemplateInitWidget_AStoryX_3_6_7Private;
	class ASERAPI ProjectTemplateInitWidget_AStoryX_3_6_7 :public YSSCore::Editor::ProjectTemplateInitWidget {
		Q_OBJECT;
	public:
		ProjectTemplateInitWidget_AStoryX_3_6_7(const YSSCore::General::YSSURI& targetUri, QWidget* parent = nullptr);
		virtual ~ProjectTemplateInitWidget_AStoryX_3_6_7();
	public slots:
		void onCreateButtonClicked();
	private:
		bool prepareProjectV3();
		void refreshWhereLabel();
	private:
		ProjectTemplateInitWidget_AStoryX_3_6_7Private* d;
	};

	class ASERAPI ProjectTemplate_AStoryX_3_6_7 :public YSSCore::Editor::ProjectTemplateProvider {
	public:
		ProjectTemplate_AStoryX_3_6_7(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~ProjectTemplate_AStoryX_3_6_7();
		virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget(
			const YSSCore::General::YSSURI& targetUri) override;
	};
}
#endif // Plugin_ASERStudio_YSS_ASERProjectTemplate_h
