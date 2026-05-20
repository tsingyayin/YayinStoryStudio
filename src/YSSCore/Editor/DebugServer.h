#ifndef YSSCore_Editor_DebugServer_h
#define YSSCore_Editor_DebugServer_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qflags.h>
#include <QtCore/qmap.h>
#include <General/PluginModule.h>
// Forward declarations
class QString;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace YSSCore::Editor {
	class EditorPlugin;
	class DebugServerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI DebugServer :public Visindigo::General::PluginModule {
		Q_OBJECT;
	public:
		enum SupportedDebugFeatureFlag :quint16 {
			Unknown = 0x0000,
			Build = 0x0001,
			Rebuild = 0x0002,
			Clean = 0x0004,
			DebugRun = 0x0008,
			Run = 0x0010,
			StepInto = 0x0020,
			StepOver = 0x0040,
			Suspend = 0x0080,
			Continue = 0x0100,
			Stop = 0x0200,
			Breakpoint = 0x0400,
			TargetOutput = 0x0800,
			TargetError = 0x1000,
			TargetStdInput = 0x2000,
		};
		Q_ENUM(SupportedDebugFeatureFlag);
		using DebugAction = SupportedDebugFeatureFlag;
		Q_DECLARE_FLAGS(SupportedDebugFeature, SupportedDebugFeatureFlag);
	signals:
		void actionStarted(DebugAction action);
		void actionPercent(DebugAction action, qint32 finished, qint32 total);
		void actionMessage(DebugAction action, const QString& message);
		void actionFinished(DebugAction action, bool success);
		void targetStdOutput(const QString& message);
		void targetStdError(const QString& message);
		void breakpointChanged(const QString& filePath, qint32 lineNumber, bool enabled);
		void breakpointHited(const QString& filePath, qint32 lineNumber);
	public:
		static QString getActionString(DebugAction action);
	public:
		DebugServer(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~DebugServer();
		void setSupportedFeatures(SupportedDebugFeature features);
		SupportedDebugFeature getSupportedFeatures();
	public:
		virtual void onBuild(bool rebuild);
		virtual void onClean(bool includeTarget);
		virtual void onRun(bool debug);
		virtual void onStepInto();
		virtual void onStepOver();
		virtual void onSuspend();
		virtual void onContinue();
		virtual void onStop();
		virtual void setBreakpoint(const QString& filePath, qint32 lineNumber, bool enabled = true);
		virtual QMap<QString, QList<qint32>> getBreakpoints();
		virtual void setBreakpoints(const QMap<QString, QList<qint32>>& breakpoints);
		virtual void writeTargetStdInput(const QString& message);
	private:
		DebugServerPrivate* d;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS(DebugServer::SupportedDebugFeature);
}

#endif // YSSCore_Editor_DebugServer_h