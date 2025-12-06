#pragma once
#include "../General/PluginModule.h"
// Forward declarations
class QString;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Editor {
	class EditorPlugin;
	class DebugData;
	class DebugFailedData;
	class DebugServerPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI DebugServer :public Visindigo::General::PluginModule{
		Q_OBJECT;
	public:
		enum class SupportedDebugFeature :quint16 {
			None = 0b0000000000000000,
			Build = 0b0000000000000001,
			Clear = 0b0000000000000010, // clear all
			Debug = 0b0000000000000100,
			Pause = 0b0000000000001000, // pause & continue & stop
			Run = 0b0000000000010000,
			Step = 0b0000000000100000, // next step
			Process = 0b0000000001000000, // next process
			All = 0b1111111111111111,
		};
	signals:
		void buildStarted();
		void buildMessage(const QString& message);
		void buildFinished(bool success);
		void debugData(const DebugData& data);
		void debugMessage(const QString& message);
		void currentExecute(const QString& filePath, quint32 lineCount);
		void debugFailed(const DebugFailedData& data);
	public:
		DebugServer(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~DebugServer();
		void setSupportedFeatures(SupportedDebugFeature features);
		SupportedDebugFeature getSupportedFeatures();
		virtual void onBuild() {};
		virtual void onClear() {};
		virtual void onDebugStart() {};
		virtual void onPause() {};
		virtual void onContinue() {};
		virtual void onStop(bool resume = false) {};
		virtual void onRun() {};
		virtual void nextStep() {};
		virtual void nextProcess() {};
		virtual QWidget* getDebugSettingsWidget(QWidget* parent = nullptr) { return nullptr; };
		Visindigo::Utility::JsonConfig* getDebugServerConfig();
	private:
		DebugServerPrivate* d;
	};
}