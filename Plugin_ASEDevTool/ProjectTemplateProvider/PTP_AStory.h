#pragma once
#include <Editor/ProjectTemplateProvider.h>
#include <Editor/ConfigWidget.h>
class PTP_AStoryInitWidget :public YSSCore::Editor::ProjectTemplateInitWidget
{
	Q_OBJECT;
	YSSCore::Editor::ConfigWidget* ConfigWidget;
public:
	PTP_AStoryInitWidget(QWidget* parent = nullptr);
};


class PTP_AStory : public YSSCore::Editor::ProjectTemplateProvider
{
public:
	PTP_AStory();
};