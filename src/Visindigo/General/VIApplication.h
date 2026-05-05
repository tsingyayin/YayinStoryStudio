#ifndef Visindigo_General_VIApplication_h
#define Visindigo_General_VIApplication_h
#include <QtCore/qcoreapplication.h>
#include <QtGui/qguiapplication.h>
#include <QtWidgets/qapplication.h>
#include "General/Exception.h"
#include "VICompileMacro.h"
namespace Visindigo::Widgets { 
	class Terminal;
}
namespace Visindigo::General {
	class VIApplicationPrivate;
	class Plugin;

	class VisindigoAPI ApplicationLoadingMessageHandler {
	public:
		virtual void onLoadingMessage(const QString& msg) = 0;
		virtual void enableHandler() = 0;
		virtual void disableHandler() = 0;
	};

	class VisindigoAPI ApplicationExceptionMessageHandler {
	public:
		virtual void onExceptionMessage(const Exception& ex) = 0;
		virtual void enableHandler() = 0;
		virtual void exec() = 0;
		virtual void disableHandler() = 0;
	};

	class VisindigoAPI VIApplication :public QObject{
		Q_OBJECT;
	public:
		enum AppType {
			CoreApp,
			GuiApp,
			WidgetApp,
		};
		Q_ENUM(AppType);
		enum EnvKey {
			LogFolderPath,
			LogFileNameTimeFormat,
			LogTimeFormat,
			PluginFolderPath,
			ConfigPath,
			MinimumLoadingTimeMS,
			UseVirtualTerminal,
			TerminalAutoShow,
			ThemeFolderPath,
			SaveCommandHistory,
			DefaultColorTheme,
		};
		Q_ENUM(EnvKey);
	public:
		static VIApplication* getInstance();
		VIApplication(int& argc, char** argv, AppType appType = WidgetApp, bool changeWorkingDirToExeDir = true);
		~VIApplication();
		void setMainPlugin(Plugin* plugin);
		Plugin* getMainPlugin() const;
		void addDependencyPlugin(Plugin* plugin);
		QList<Plugin*> getDependencyPlugins() const;
		static void setEnvConfig(EnvKey key, const QVariant& value);
		static QVariant getEnvConfig(EnvKey key);
		void onException(const Exception& ex);
		void setLoadingMessageHandler(ApplicationLoadingMessageHandler* handler);
		void setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler);
		void setGlobalFont(const QString& fontPath, int fontID = 0);
		int start();
		bool applicationStarted() const;
		Widgets::Terminal* getVirtualTerminal() const;
	private:
		VIApplicationPrivate* d;
	};
}
#define VISetEnv(key, value) Visindigo::General::VIApplication::setEnvConfig(key, value)
#define VIApp Visindigo::General::VIApplication::getInstance()

#endif // Visindigo_General_VIApplication_h