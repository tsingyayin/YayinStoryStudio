#pragma once
#include <Editor/ProjectTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class ASEAStoryPTIW :public YSSCore::Editor::ProjectTemplateInitWidget
{
	Q_OBJECT;
	YSSCore::Widgets::ConfigWidget* ConfigWidget;
	QVBoxLayout* Layout;
public:
	ASEAStoryPTIW(QWidget* parent = nullptr);
	virtual void resizeEvent(QResizeEvent* event) override;
};


class ASEAStoryPTP : public YSSCore::Editor::ProjectTemplateProvider
{
public:
	ASEAStoryPTP(YSSCore::Editor::EditorPlugin* plugin);
	virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
};