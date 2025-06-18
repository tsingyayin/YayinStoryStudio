#include "../ASEAStoryPTP.h"
#include <QtWidgets/qboxlayout.h>
#include <General/TranslationHost.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
ASEAStoryPTIW::ASEAStoryPTIW(QWidget* parent)
	: YSSCore::Editor::ProjectTemplateInitWidget(parent)
{
	//this->setFixedSize(800, 600);
	this->setWindowTitle(YSSTR("ASEDevTool::provider.window.title"));
	ConfigWidget = new YSSCore::Widgets::ConfigWidget(this);
	ConfigWidget->loadCWJson(YSSCore::Utility::FileUtility::readAll( ":/plugin/compiled/ASEDevTool/configWidget/PTP.json"));
	Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);
	Layout->addWidget(ConfigWidget);
	// Constructor implementation
}
void ASEAStoryPTIW::resizeEvent(QResizeEvent* event)
{
	YSSCore::Editor::ProjectTemplateInitWidget::resizeEvent(event);
	
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