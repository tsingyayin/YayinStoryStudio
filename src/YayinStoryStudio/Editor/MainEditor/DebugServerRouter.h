#ifndef YSS_MainEditor_DebugServerRouter_h
#define YSS_MainEditor_DebugServerRouter_h
#include <Editor/DebugServer.h>
#include <Editor/DebugServerManager.h>
namespace YSS::Editor {
	class DebugServerRouterPrivate;
	class DebugServerRouter :public QObject {
		Q_OBJECT;
	signals:
		void debugServerChanged();
		void actionStarted(YSSCore::Editor::DebugServer::DebugAction action);
		void actionPercent(YSSCore::Editor::DebugServer::DebugAction action, qint32 finished, qint32 total);
		void actionMessage(YSSCore::Editor::DebugServer::DebugAction action, const QString& message);
		void actionFinished(YSSCore::Editor::DebugServer::DebugAction action, bool success);
		void targetStdOutput(const QString& message);
		void targetStdError(const QString& message);
		void breakpointChanged(const QString& filePath, qint32 lineNumber, bool enabled);
		void breakpointHited(const QString& filePath, qint32 lineNumber);
	private:
		DebugServerRouter();
	public:
		static QString getActionString(YSSCore::Editor::DebugServer::DebugAction action);
		static DebugServerRouter* getInstance();
		~DebugServerRouter();
	public:
		bool testDebugFeature(YSSCore::Editor::DebugServer::SupportedDebugFeature feature);
		bool build(bool rebuild = true);
		bool clean(bool includeTarget = true);
		bool run(bool debug = true);
		bool stepInto();
		bool stepOver();
		bool suspend();
		bool contiune();
		bool stop();
		bool restart();
		bool setBreakpoint(const QString& filePath, qint32 lineNumber, bool enabled = true);
		QMap<QString, QList<qint32>> getBreakpoints();
		bool setBreakpoints(const QMap<QString, QList<qint32>>& breakpoints);
		bool writeTargetStdInput(const QString& message);
	private:
		DebugServerRouterPrivate* d;
	};
}
#define YSSDSR YSS::Editor::DebugServerRouter::getInstance()

#endif // YSS_MainEditor_DebugServerRouter_h
