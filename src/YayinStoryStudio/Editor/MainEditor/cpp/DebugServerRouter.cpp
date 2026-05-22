#include "Editor/MainEditor/DebugServerRouter.h"
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include <QtWidgets/qmessagebox.h>
#include "Editor/MainEditor/MainWin.h"
#include <General/Log.h>
#include <General/TranslationHost.h>

namespace YSS::Editor {
	class DebugServerRouterPrivate {
		friend class DebugServerRouter;
	protected:
		static DebugServerRouter* Instance;
		YSSCore::Editor::DebugServer* CurrentServer = nullptr;
		bool lastDebugStart = false;
		YSSCore::Editor::DebugServer* getCurrentServer() {
			if (not CurrentServer) {
				QString debugServerID = YSS::GlobalValue::getCurrentProject()->getProjectDebugServerID();
				if (debugServerID.isEmpty()) {
					QMessageBox::warning(YSS::Editor::MainWin::getInstance(),
						VITR("YSS::editor.debug.notSelected.title"), VITR("YSS::editor.debug.notSelected.message"));
					vgErrorF << "No debug server set for current project!";
					return nullptr;
				}
				CurrentServer = YSSDSM->getDebugServer(debugServerID);
				if (CurrentServer) {
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::actionStarted,
						Instance, &DebugServerRouter::actionStarted);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::actionPercent,
						Instance, &DebugServerRouter::actionPercent);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::actionMessage,
						Instance, &DebugServerRouter::actionMessage);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::actionFinished,
						Instance, &DebugServerRouter::actionFinished);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::targetStdOutput,
						Instance, &DebugServerRouter::targetStdOutput);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::targetStdError,
						Instance, &DebugServerRouter::targetStdError);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::breakpointChanged,
						Instance, &DebugServerRouter::breakpointChanged);
					QObject::connect(CurrentServer, &YSSCore::Editor::DebugServer::breakpointHited,
						Instance, &DebugServerRouter::breakpointHited);
					Instance->debugServerChanged();
				}
				else {
					QMessageBox::warning(YSS::Editor::MainWin::getInstance(),
						VITR("YSS::editor.debug.notFound.title"), VITR("YSS::editor.debug.notFound.message").arg(debugServerID));
					vgErrorF << "Debug server" << debugServerID << "not found!";
				}
			}
			return CurrentServer;
		}
	};

	DebugServerRouter* DebugServerRouterPrivate::Instance = nullptr;

	DebugServerRouter::DebugServerRouter() {
		d = new DebugServerRouterPrivate();
		DebugServerRouterPrivate::Instance = this;
		connect(YSSDSM, &YSSCore::Editor::DebugServerManager::projectDebugServerChanged, this, [this](const QString& serverID) {
			if (d->CurrentServer) {
				disconnect(d->CurrentServer, nullptr, this, nullptr);
			}
			d->CurrentServer = nullptr; // auto re-get in d->getCurrentServer()
			});
		connect(this, &DebugServerRouter::actionStarted, this, [this](YSSCore::Editor::DebugServer::DebugAction action) {
			vgInfoF << "Debug action started:" << getActionString(action);
			});
		connect(this, &DebugServerRouter::actionFinished, this, [this](YSSCore::Editor::DebugServer::DebugAction action, bool success) {
			vgInfoF << "Debug action finished:" << getActionString(action) << "Success:" << success;
			});
		connect(this, &DebugServerRouter::actionMessage, this, [this](YSSCore::Editor::DebugServer::DebugAction action, const QString& message) {
			vgInfoF << "Debug action message:" << getActionString(action) << message;
			});
		connect(this, &DebugServerRouter::actionPercent, this, [this](YSSCore::Editor::DebugServer::DebugAction action, qint32 finished, qint32 total) {
			vgInfoF << "Debug action percent:" << getActionString(action) << "Finished:" << finished << "Total:"
				<< total << "Percent:" << (total > 0 ? QString::number((double)finished / total * 100, 'f', 2) + "%" : "N/A");
			});

		connect(this, &DebugServerRouter::targetStdOutput, this, [this](const QString& message) {
			vgInfoF << "Debug target std output:" << message;
			});
		connect(this, &DebugServerRouter::targetStdError, this, [this](const QString& message) {
			vgErrorF << "Debug target std error:" << message;
			});
	}

	DebugServerRouter* DebugServerRouter::getInstance() {
		if (not DebugServerRouterPrivate::Instance) {
			DebugServerRouterPrivate::Instance = new DebugServerRouter();
		}
		return DebugServerRouterPrivate::Instance;
	}

	DebugServerRouter::~DebugServerRouter() {
		delete d;
	}

	QString DebugServerRouter::getActionString(YSSCore::Editor::DebugServer::DebugAction action) {
		return YSSCore::Editor::DebugServer::getActionString(action);
	}

	bool DebugServerRouter::testDebugFeature(YSSCore::Editor::DebugServer::SupportedDebugFeature feature) {
		auto server = d->getCurrentServer();
		if (server) {
			return (server->getSupportedFeatures() & feature) != 0;
		}
		return false;
	}

	bool DebugServerRouter::build(bool rebuild) {
		if (not testDebugFeature(rebuild ? YSSCore::Editor::DebugServer::Rebuild : YSSCore::Editor::DebugServer::Build)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onBuild(rebuild);
		}
		return true;
	}

	bool DebugServerRouter::clean(bool includeTarget) {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Clean)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onClean(includeTarget);
		}
		return true;
	}

	bool DebugServerRouter::run(bool debug) {
		if (not testDebugFeature(debug ? YSSCore::Editor::DebugServer::DebugRun : YSSCore::Editor::DebugServer::Run)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			d->lastDebugStart = debug;
			server->onRun(debug);
		}
		return true;
	}

	bool DebugServerRouter::stepInto() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::StepInto)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onStepInto();
		}
		return true;
	}

	bool DebugServerRouter::stepOver() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::StepOver)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onStepOver();
		}
		return true;
	}

	bool DebugServerRouter::suspend() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Suspend)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onSuspend();
		}
		return true;
	}

	bool DebugServerRouter::contiune() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Continue)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onContinue();
		}
		return true;
	}

	bool DebugServerRouter::stop() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Stop)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onStop();
		}
		return true;
	}

	bool DebugServerRouter::restart() {
		if (not testDebugFeature(d->lastDebugStart ? YSSCore::Editor::DebugServer::DebugRun : YSSCore::Editor::DebugServer::Run)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->onStop();
			server->onRun(d->lastDebugStart);
		}
		return true;
	}

	bool DebugServerRouter::setBreakpoint(const QString& filePath, qint32 lineNumber, bool enabled) {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Breakpoint)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->setBreakpoint(filePath, lineNumber, enabled);
		}
		return true;
	}

	QMap<QString, QList<qint32>> DebugServerRouter::getBreakpoints() {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Breakpoint)) {
			return {};
		}
		auto server = d->getCurrentServer();
		if (server) {
			return server->getBreakpoints();
		}
		return {};
	}

	bool DebugServerRouter::setBreakpoints(const QMap<QString, QList<qint32>>& breakpoints) {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::Breakpoint)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->setBreakpoints(breakpoints);
		}
		return true;
	}

	bool DebugServerRouter::writeTargetStdInput(const QString& message) {
		if (not testDebugFeature(YSSCore::Editor::DebugServer::TargetStdInput)) {
			return false;
		}
		auto server = d->getCurrentServer();
		if (server) {
			server->writeTargetStdInput(message);
		}
		return true;
	}
}