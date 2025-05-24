#include "../PTP_AStory.h"
#include <General/TranslationHost.h>

PTP_AStoryInitWidget::PTP_AStoryInitWidget(QWidget* parent)
	: YSSCore::Editor::ProjectTemplateInitWidget(parent)
{
	// Constructor implementation
}
PTP_AStory::PTP_AStory()
{
	setTemplateIcon(QIcon(":/plugin/compiled/AStory/icon/provider.png"));
	setTemplateID("ASEProject");
	setTemplateName(YSSTR("AStory::template.name"));
	setTemplateDescription(YSSTR("AStory::template.description"));
	// Constructor implementation
}