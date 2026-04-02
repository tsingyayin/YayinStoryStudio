#ifndef ASERStudio_YSS_ASERFileTemplate_h
#define ASERStudio_YSS_ASERFileTemplate_h
#include "ASERStudioCompileMacro.h"
#include <Editor/FileTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

namespace Visindigo::General {
	class YSSProject;
}

namespace ASERStudio::YSS {
	class FileTemplateInitWidget_AStoryXPrivate;
	class ASERAPI FileTemplateInitWidget_AStoryX :public YSSCore::Editor::FileTemplateInitWidget
	{
		Q_OBJECT;
	public:
		FileTemplateInitWidget_AStoryX(const QString& initFolder, QWidget* parent = nullptr);
		virtual ~FileTemplateInitWidget_AStoryX();
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onLineEditTextChanged(const QString& node, const QString& text);
		void onCreateButtonClicked();
		QStringList initFileV3();
	private:
		void refreshWhereLabel();
	private:
		FileTemplateInitWidget_AStoryXPrivate* d;
	};

	class ASERAPI FileTemplate_AStoryX : public YSSCore::Editor::FileTemplateProvider
	{
	public:
		FileTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~FileTemplate_AStoryX();
		virtual YSSCore::Editor::FileTemplateInitWidget* fileInitWidget(const QString& initPath) override;
	};
}
#endif // ASERStudio_YSS_ASERFileTemplate_h