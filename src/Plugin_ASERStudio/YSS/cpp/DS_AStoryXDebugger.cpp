#include "YSS/DS_AStoryXDebugger.h"
#include <General/Log.h>
#include "Plugin_ASERStudio.h"
#include "ASEREnv/ASERDebugIO.h"
#include "ASEREnv/ASERProgram.h"
#include <General/YSSProject.h>
#include <Utility/JsonConfig.h>

namespace ASERStudio::YSS {
	DS_AStoryXDebugger::DS_AStoryXDebugger(YSSCore::Editor::EditorPlugin* plugin) : 
		YSSCore::Editor::DebugServer("AStoryXDebugger", ASERStudio_AStoryXDebuggerID, plugin) {
		this->setSupportedFeatures(DebugServer::SupportedDebugFeatureFlag::Debug |
			DebugServer::SupportedDebugFeatureFlag::Run | DebugServer::SupportedDebugFeatureFlag::Stop);
	}
	DS_AStoryXDebugger::~DS_AStoryXDebugger() {
	}
	void DS_AStoryXDebugger::onDebugStart() {
		vgDebug << "Debug Start";
		auto program = ASERStudio::Main::getInstance()->getASERProgram();
		auto debugIO = ASERStudio::Main::getInstance()->getASERDebugIO();
		auto param = ASERStudio::ASEREnv::ASERProgramLaunchParameter();
		auto path = YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder();
		auto fileName = YSSCore::General::YSSProject::getCurrentProject()->getFocusedFileName().replace(".astoryx", "");
		param.setPath(path);
		param.setFileName(fileName);
		auto sizeMode = ASERStudio::Main::getInstance()->getPluginConfig()->getString("ASERExeSettings.WindowSize");
		param.setSizeMode((ASERStudio::ASEREnv::ASERProgramLaunchParameter::SizeMode)sizeMode.toInt());
		vgDebug << program->getExecutablePath();
		program->start(param);
	}
	void DS_AStoryXDebugger::onRun() {
		onDebugStart(); // currently no difference.
	}
	void DS_AStoryXDebugger::onStop(bool resume) {
		vgDebug << "Stop";
		auto program = ASERStudio::Main::getInstance()->getASERProgram();
		if (program->isRunning()) {
			program->stop();
		}
	}
}