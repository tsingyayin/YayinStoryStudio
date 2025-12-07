#include "../DS_ASE.h"
#include "../ASELaunchArgu.h"
#include <QtCore/qprocess.h>
#include <Utility/FileUtility.h>
#include <General/Log.h>
#include <General/YSSProject.h>
class DS_ASEPrivate {
public:
	ASELaunchArgu launchArgu;
	QProcess* ASEProgram = nullptr;
	QString ASEExecutablePath = "/resource/third_party/ASE 2.05.22.1A/Arknights_StoryEditor.exe";
};
DS_ASE::DS_ASE(Visindigo::Editor::EditorPlugin* plugin)
	: Visindigo::Editor::DebugServer("ASEDevTool Debug Server", "ASEDevTool_AStory", plugin), d(new DS_ASEPrivate()){
}

DS_ASE::~DS_ASE() {
	delete d;
}

void DS_ASE::onBuild() {
	// Nothing to do
}

void DS_ASE::onClear() {
	// Nothing to do
}

void DS_ASE::onDebugStart() {
	if (!Visindigo::General::YSSProject::getCurrentProject()->getFocusedFileName().endsWith(".astory")) {
		yWarningF << "The focused file is not an .astory file.";
		return;
	}
	if (d->ASEProgram != nullptr) {
		if (d->ASEProgram->state() == QProcess::ProcessState::Running) {
			emit debugMessage("ASE is already running.");
			return;
		}
		else {
			d->ASEProgram->deleteLater();
			d->ASEProgram = nullptr;
		}
	}
	d->ASEProgram = new QProcess();
	connect(d->ASEProgram, &QProcess::readyReadStandardOutput, this, &DS_ASE::onASEStdOutput);
	connect(d->ASEProgram, &QProcess::readyReadStandardError, this, &DS_ASE::onASEErrOutput);
	QStringList arguments;
	d->launchArgu.setWorkingFolder(Visindigo::General::YSSProject::getCurrentProject()->getProjectFolder());
	d->launchArgu.setMainFileName(Visindigo::General::YSSProject::getCurrentProject()->getFocusedFileName());
	arguments << d->launchArgu.toString();
	yDebugF << Visindigo::Utility::FileUtility::getProgramPath() + d->ASEExecutablePath;
	yDebugF << arguments;
	d->ASEProgram->start(Visindigo::Utility::FileUtility::getProgramPath() + d->ASEExecutablePath, arguments);
	if (!d->ASEProgram->waitForStarted(5000)) {
		yDebugF << "ASE launch failed.";
		delete d->ASEProgram;
		d->ASEProgram = nullptr;
		return;
	}
}

void DS_ASE::onPause() {
	// Nothing to do
}

void DS_ASE::onContinue() {
	// Nothing to do
}

void DS_ASE::onStop(bool resume) {
	if (d->ASEProgram == nullptr) {
		emit debugMessage("ASE is not running.");
		return;
	}
	d->ASEProgram->terminate();
	if (!d->ASEProgram->waitForFinished(5000)) {
		d->ASEProgram->kill();
		d->ASEProgram->waitForFinished();
	}
	delete d->ASEProgram;
	d->ASEProgram = nullptr;
}

void DS_ASE::onRun() {
	// Nothing to do
}

void DS_ASE::nextStep() {
	// Nothing to do
}

void DS_ASE::nextProcess() {
	// Nothing to do
}

QWidget* DS_ASE::getDebugSettingsWidget(QWidget* parent) {
	return nullptr;
}

void DS_ASE::onASEStdOutput() {
	if (d->ASEProgram == nullptr) return;
	QString output = d->ASEProgram->readAllStandardOutput();
	yDebugF << "[ASE Std Out]" << output;
}

void DS_ASE::onASEErrOutput() {
	if (d->ASEProgram == nullptr) return;
	QString output = d->ASEProgram->readAllStandardError();
	yDebugF << "[ASE Err Out]" << output;
}

void DS_ASE::ASEStdInput(const QString& input) {
	if (d->ASEProgram == nullptr) return;
	d->ASEProgram->write(input.toUtf8());
}