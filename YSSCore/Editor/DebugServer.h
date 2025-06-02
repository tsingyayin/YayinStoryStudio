#pragma once
#include <QtGui>
#include "../Macro.h"

namespace YSSCore::Editor {
	class DebugData;
	class DebugFailedData;
	class DebugServerPrivate;
	class YSSCoreAPI DebugServer {
	signals:
		void buildStarted();
		void buildMessage(const QString& message);
		void buildFinished(bool success);
		void debugData(const DebugData& data);
		void currentExecute(const QString& filePath, quint32 lineCount);
		void debugFailed(const DebugFailedData& data);
	public:
		DebugServer();
		~DebugServer();
	public slots:
		virtual void onBuild() = 0;
		virtual void onDebug() = 0;
		virtual void onRun() = 0;
		virtual void nextStep() = 0;
		virtual void nextProcess() = 0;
	private:
		DebugServerPrivate* d;
	};
}