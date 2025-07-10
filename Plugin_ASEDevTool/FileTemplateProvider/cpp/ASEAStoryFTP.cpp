#include "../ASEAStoryFTP.h"
#include <QtWidgets/qboxlayout.h>
#include <General/TranslationHost.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qpushbutton.h>
#include <Utility/JsonConfig.h>
#include <QtWidgets/qlabel.h>
#include <General/YSSProject.h>
#include <QtWidgets/qmessagebox.h>
ASEAStoryFTIW::ASEAStoryFTIW(QWidget* parent)
	: YSSCore::Editor::FileTemplateInitWidget(parent)
{
}

ASEAStoryFTP::ASEAStoryFTP(YSSCore::Editor::EditorPlugin* plugin) :
	FileTemplateProvider("ASE AStory File Template Provider", "ASEAStoryFTP", plugin)
{
	setTemplateIconPath(":/plugin/compiled/ASEDevTool/icon/ASEA_Dark.png");
	setTemplateID("ASEAStoryFile");
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

YSSCore::Editor::FileTemplateInitWidget* ASEAStoryFTP::fileInitWidget() {
	return new ASEAStoryFTIW();
}