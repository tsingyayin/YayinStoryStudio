#include "YSS/DS_AStoryXDebugger.h"
#include <General/Log.h>
#include "Plugin_ASERStudio.h"
#include "ASEREnv/ASERDebugIO.h"
#include "ASEREnv/ASERProgram.h"
#include <General/YSSProject.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
#include <QtGui/qwindow.h>
namespace ASERStudio::YSS {
	class DS_AStoryXDebuggerPrivate {
		friend class DS_AStoryXDebugger;
	protected:
		bool signalConnected = false;
		bool isDebugMode = false;
		bool isRunning = false;
		QString launchFileName;
	};
	DS_AStoryXDebugger::DS_AStoryXDebugger(YSSCore::Editor::EditorPlugin* plugin) : 
		YSSCore::Editor::DebugServer("AStoryXDebugger", ASERStudio_AStoryXDebuggerID, plugin) {
		d = new DS_AStoryXDebuggerPrivate;
		this->setSupportedFeatures(
			DebugRun | Run | Stop | Suspend | Continue
		);
		
	}
	DS_AStoryXDebugger::~DS_AStoryXDebugger() {
	}
	void DS_AStoryXDebugger::onRun(bool debug) {
		auto launchFileName = YSSCore::General::YSSProject::getCurrentProject()->getFocusedFileName();
		if (not launchFileName.endsWith(".astoryx", Qt::CaseInsensitive)) {
			QMessageBox::critical(nullptr, VITR("ASERStudio::debugger.notLegalFile.title"),
				VITR("ASERStudio::debugger.notLegalFile.message").arg(launchFileName));
			return;
		}
		d->launchFileName = launchFileName.left(launchFileName.length() - 8);
		this->onStop();
		if (not d->signalConnected) {
			connect(ASERStudio::Main::getInstance()->getASERProgram(), &ASERStudio::ASEREnv::ASERProgram::programStopped, this, [this](qint64 exitCode) {
				if (d->isRunning) {
					emit actionFinished(d->isDebugMode ? DebugRun : Run, exitCode == 0);
				}
				d->isRunning = false;
				});
			connect(ASERStudio::Main::getInstance()->getASERProgram(), &ASERStudio::ASEREnv::ASERProgram::programStarted, this, [this]() {
				if (d->isRunning) {
					emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.waitingForPipe"));
				}
				});
			connect(ASERStudio::Main::getInstance()->getASERProgram(), &ASERStudio::ASEREnv::ASERProgram::namedPipeConnected, this, [this]() {
				if (d->isRunning) {
					emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.waitingForOBMI"));
					ASERStudio::Main::getInstance()->getASERProgram()->getProcessWindow()->setFlags(Qt::WindowStaysOnTopHint);
				}
				});
			connect(ASERStudio::Main::getInstance()->getASERDebugIO(), &ASERStudio::ASEREnv::ASERDebugIO::officialBundleManagerInitializeChanged,
				this, [this](ASERStudio::ASEREnv::ASERDebugIO::OfficialBundleManagerInitializeState state) {
					if (d->isRunning) {
						emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.waitingForPlay"));
						ASERStudio::Main::getInstance()->getASERDebugIO()->play(d->launchFileName);
					}
				});
			connect(ASERStudio::Main::getInstance()->getASERDebugIO(), &ASERStudio::ASEREnv::ASERDebugIO::storyLoading, this, [this](const QString& name) {
				if (d->isRunning) {
					emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.loading").arg(name));
				}
				});
			connect(ASERStudio::Main::getInstance()->getASERDebugIO(), &ASERStudio::ASEREnv::ASERDebugIO::storyStarted, this, [this](const QString& name) {
				if (d->isRunning) {
					emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.started").arg(name));
				}
				});
			connect(ASERStudio::Main::getInstance()->getASERDebugIO(), &ASERStudio::ASEREnv::ASERDebugIO::storyExited, this, [this](const QString& name) {
				if (d->isRunning) {
					emit actionMessage(d->isDebugMode ? DebugRun : Run, VITR("ASERStudio::debugger.executing.exited").arg(name));
				}
				});
			d->signalConnected = true;
		}
		emit actionStarted(debug ? DebugRun : Run);
		d->isRunning = true;
		d->isDebugMode = debug;
		auto program = ASERStudio::Main::getInstance()->getASERProgram();
		bool exists = Visindigo::Utility::FileUtility::isFileExist(program->getExecutablePath());
		if (not exists) {
			QMessageBox::critical(nullptr, VITR("ASERStudio::debugger.executableNotFound.title"),
				VITR("ASERStudio::debugger.executableNotFound.message").arg(program->getExecutablePath()));
			emit actionMessage(debug ? DebugRun : Run, VITR("ASERStudio::debugger.executableNotFound.message").arg(program->getExecutablePath()));
			emit actionFinished(debug ? DebugRun : Run, false);
			return;
		}
		vgDebug << "Debug Start";
		
		auto debugIO = ASERStudio::Main::getInstance()->getASERDebugIO();
		auto param = ASERStudio::ASEREnv::ASERProgramLaunchParameter();
		auto path = YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder();
		param.setProjectPath(path);
		auto sizeMode = ASERStudio::Main::getInstance()->getPluginConfig()->getString("ASERExeSettings.WindowSize");
		param.setSizeMode((ASERStudio::ASEREnv::ASERProgramLaunchParameter::SizeMode)sizeMode.toInt());
		emit actionMessage(debug ? DebugRun : Run, VITR("ASERStudio::debugger.executing.waitingForLaunch"));
		program->start(param);
	}

	void DS_AStoryXDebugger::onStop() {
		vgDebug << "Stop";
		auto program = ASERStudio::Main::getInstance()->getASERProgram();
		if (program->isRunning()) {
			program->waitStop(10000);
		}
	}
}