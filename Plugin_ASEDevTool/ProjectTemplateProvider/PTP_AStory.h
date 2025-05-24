#pragma once
#include <Editor/ProjectTemplateProvider.h>

class PTP_AStoryInitWidget :public YSSCore::Editor::ProjectTemplateInitWidget
{
	Q_OBJECT;
public:
	PTP_AStoryInitWidget(QWidget* parent = nullptr);
};


class PTP_AStory : public YSSCore::Editor::ProjectTemplateProvider
{
public:
	PTP_AStory();
};