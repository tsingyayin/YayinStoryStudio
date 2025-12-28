#include "../DS_ASE.h"
#include "../ASELaunchArgu.h"
#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>
#include <Utility/FileUtility.h>
#include <General/Log.h>
#include <General/YSSProject.h>
#include <QtNetwork/qlocalsocket.h>
#include <Editor/FileServerManager.h>

class DS_ASEPrivate {
public:
	ASELaunchArgu launchArgu;
	QProcess* ASEProgram = nullptr;
	QString ASEExecutablePath = "/resource/third_party/ASE 2.05.22.1A/Arknights_StoryEditor.exe";
	QLocalSocket* ASEDebugPipe = nullptr;
};
DS_ASE::DS_ASE(YSSCore::Editor::EditorPlugin* plugin)
	: YSSCore::Editor::DebugServer("ASEDevTool Debug Server", "ASEDevTool_AStory", plugin), d(new DS_ASEPrivate()){
	setSupportedFeatures(DebugServer::Debug | DebugServer::Stop);
	d->ASEDebugPipe = new QLocalSocket();
	connect(d->ASEDebugPipe, &QLocalSocket::readyRead, this, &DS_ASE::onASEPipeOut);
	connect(d->ASEDebugPipe, &QLocalSocket::disconnected, [this]() {
		vgDebugF << "ASE debug pipe disconnected.";
		d->ASEDebugPipe->deleteLater();
		d->ASEDebugPipe = nullptr;
		});
	connect(d->ASEDebugPipe, &QLocalSocket::connected, [this]() {
		vgDebugF << "ASE debug pipe connected.";
		ASEPipeInput("test hello\n");
		vgDebugF << "Hello message send";
		});
	connect(d->ASEDebugPipe, &QLocalSocket::errorOccurred, [this](QLocalSocket::LocalSocketError socketError) {
		vgDebugF << "ASE debug pipe error:" << socketError;
		});
	d->ASEDebugPipe->setServerName("ASEDebugPipe");
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
	if (!YSSCore::General::YSSProject::getCurrentProject()->getFocusedFileName().endsWith(".astory")) {
		vgWarningF << "The focused file is not an .astory file.";
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
	d->launchArgu.setWorkingFolder(YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder());
	d->launchArgu.setMainFileName(YSSCore::General::YSSProject::getCurrentProject()->getFocusedFileName());
	arguments << d->launchArgu.toString();
	vgDebugF << Visindigo::Utility::FileUtility::getProgramPath() + d->ASEExecutablePath;
	vgDebugF << arguments;
	d->ASEProgram->start(Visindigo::Utility::FileUtility::getProgramPath() + d->ASEExecutablePath, arguments);
	if (!d->ASEProgram->waitForStarted(5000)) {
		vgDebugF << "ASE launch failed.";
		delete d->ASEProgram;
		d->ASEProgram = nullptr;
		return;
	}
	d->ASEDebugPipe->disconnectFromServer();
	QTimer* timer = new QTimer();
	timer->setInterval(3000);
	connect(timer, &QTimer::timeout, [this, timer]() {
		d->ASEDebugPipe->connectToServer(QIODevice::ReadWrite);
		if (d->ASEDebugPipe->state() == QLocalSocket::ConnectedState) {
			timer->stop();
			timer->deleteLater();
		}
		else {
			vgDebugF << "Trying to connect ASE debug pipe...";
		}
		});
	timer->start();
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
	if (d->ASEProgram != nullptr) {
		d->ASEProgram->deleteLater();
		d->ASEProgram = nullptr;
	}
	if (d->ASEDebugPipe != nullptr) {
		d->ASEDebugPipe->deleteLater();
		d->ASEDebugPipe = nullptr;
	}
}

void DS_ASE::onRun() {
	
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
	vgDebugF << "[ASE Std Out]" << output;
}

void DS_ASE::onASEErrOutput() {
	if (d->ASEProgram == nullptr) return;
	QString output = d->ASEProgram->readAllStandardError();
	vgDebugF << "[ASE Err Out]" << output;
}

void DS_ASE::ASEStdInput(const QString& input) {
	if (d->ASEProgram == nullptr) return;
	d->ASEProgram->write(input.toUtf8());
}

void DS_ASE::onASEPipeOut() {
	if (d->ASEDebugPipe == nullptr) return;
	QString output = QString::fromUtf8(d->ASEDebugPipe->readAll());
	QStringList commands = output.split('\n', Qt::SkipEmptyParts);
	for (const QString& cmd : commands) {
		// Process command here
		vgDebugF << "[ASE Pipe Command]" << cmd;
		if (cmd.startsWith("rawIndex ")) {
			quint32 lineNumber = cmd.mid(QString("rawIndex ").length()).toUInt() + 1; // i dont know why, may be ase need to fix.
			QString filePath = d->launchArgu.getWorkingFolder() + "/Stories/" + d->launchArgu.getMainFileName();
			vgDebugF << "Switching to line" << lineNumber << "in file" << filePath;
			emit YSSFSM->switchLineEdit(filePath, lineNumber);
		}
		else if (cmd.startsWith("ended")) {
			//onStop();
		}
	}
}

void DS_ASE::ASEPipeInput(const QString& input) {
	if (d->ASEDebugPipe == nullptr) return;
	d->ASEDebugPipe->write(input.toUtf8());
}