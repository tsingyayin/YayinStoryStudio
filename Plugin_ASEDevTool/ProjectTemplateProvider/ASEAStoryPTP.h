#pragma once
#include <Editor/ProjectTemplateProvider.h>
#include <Editor/ConfigWidget.h>
class ASEAStoryPTIW :public YSSCore::Editor::ProjectTemplateInitWidget
{
	Q_OBJECT;
	YSSCore::Editor::ConfigWidget* ConfigWidget;
public:
	ASEAStoryPTIW(QWidget* parent = nullptr);
};


class ASEAStoryPTP : public YSSCore::Editor::ProjectTemplateProvider
{
public:
	ASEAStoryPTP(YSSCore::Editor::EditorPlugin* plugin);
	virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
};