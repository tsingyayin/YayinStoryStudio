#include "../ASEAStoryPTP.h"
#include <General/TranslationHost.h>
#include <Editor/ConfigWidget.h>
#include <Utility/FileUtility.h>
ASEAStoryPTIW::ASEAStoryPTIW(QWidget* parent)
	: YSSCore::Editor::ProjectTemplateInitWidget(parent)
{
	this->setFixedSize(800, 600);
	this->setWindowTitle(YSSTR("ASEDevTool::provider.window.title"));
	ConfigWidget = new YSSCore::Editor::ConfigWidget(this);
	ConfigWidget->loadCWJson(YSSCore::Utility::FileUtility::readAll( ":/plugin/compiled/ASEDevTool/configWidget/PTP.json"));
	// Constructor implementation
}
void ASEAStoryPTIW::resizeEvent(QResizeEvent* event)
{
	YSSCore::Editor::ProjectTemplateInitWidget::resizeEvent(event);
	if (ConfigWidget) {
		ConfigWidget->setGeometry(0, 0, this->width(), this->height());
	}
}
ASEAStoryPTP::ASEAStoryPTP(YSSCore::Editor::EditorPlugin* plugin):ProjectTemplateProvider(plugin)
{
	setTemplateIcon(QIcon(":/plugin/compiled/ASEDevTool/icon/provider.png"));
	setTemplateID("ASEAStoryProject");
	setTemplateName(YSSTR("ASEDevTool::provider.astory.name"));
	setTemplateDescription(YSSTR("ASEDevTool::provider.astory.description"));
	setTemplateTags(QStringList({
			YSSTR("ASEDevTool::provider.astory.tags.ase"),
			YSSTR("ASEDevTool::provider.astory.tags.astory"),
			YSSTR("ASEDevTool::provider.astory.tags.engine-native"),
			YSSTR("ASEDevTool::provider.astory.tags.script"),
			YSSTR("ASEDevTool::provider.astory.tags.arknights")
		}));
	// Constructor implementation
}

YSSCore::Editor::ProjectTemplateInitWidget* ASEAStoryPTP::projectInitWidget(){
	return new ASEAStoryPTIW();
}