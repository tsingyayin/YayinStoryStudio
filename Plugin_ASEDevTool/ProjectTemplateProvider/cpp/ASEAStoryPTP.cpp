#include "../ASEAStoryPTP.h"
#include <General/TranslationHost.h>

ASEAStoryPTIW::ASEAStoryPTIW(QWidget* parent)
	: YSSCore::Editor::ProjectTemplateInitWidget(parent)
{
	// Constructor implementation
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