#include "Macro.h"
#include "Plugin_ASEDevTool.h"

#ifdef AStory_EXPORT
extern "C" AStoryAPI YSSCore::Editor::EditorPlugin* YSSPluginDllMain(void)
{
	return new Plugin_ASEDevTool();
}
#endif

#ifdef AStory_DEVELOPMENT
#include <QtCore/qcoreapplication.h>
#include "LangServer/ASRuleAdaptor.h"
#include "LangServer/AStoryControllerParseData.h"
#include <General/Log.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	ASRuleAdaptor Adaptor(":/plugin/compiled/ASEDevTool/template/2.05.22.1A/BaseRule.asrule",
		":/plugin/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json");
	QStringList Story = YSSCore::Utility::FileUtility::readLines("E:/source/repos/YayinStoryStudio/x64/Release/resource/repos/ASE_Story_Project_3/Stories/深蓝碎片.astory");
	Story = Story.mid(2);
	for (auto Line : Story) {
		yDebug << "Line: " << Line;
		yDebug << "Parse Result: " << Adaptor.parse(Line);
		yDebug << "----------------------------------------";
	}
	/*AStoryController controller;
	YSSCore::Utility::JsonConfig config;
	config.parse(YSSCore::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json"));
	controller.setControllerName("avg");
	controller.setControllerMeta(config);
	controller.setControllerASRule("立绘（角色名：__pcname__，插槽编号：__slot__，差分名：__diff__，效果：__ishide__，淡入时间：__fadeintime__，淡出时间：__fadeouttime__，阻塞：__isblock__，起始位置：__startpos__，结束位置：__endpos__，移动时间：__movetime__，移动曲线：__movecurve__）");
	yDebug << controller.parse("立绘（角色名：阿米娅&{，插槽编号：}，插槽编号：1，差分名：default，效果：true，淡入时间：0.5，淡出时间：0.5，阻塞：false，起始位置：0，结束位置：100，移动时间：1.0，移动曲线：linear）");*/
	YSSCore::General::LoggerManager::getInstance()->finalSave();
	return 0;
}
#endif