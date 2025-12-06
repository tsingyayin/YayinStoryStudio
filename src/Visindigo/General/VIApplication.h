#pragma once
#include <QtCore/qcoreapplication.h>
#include <QtGui/qguiapplication.h>
#include <QtWidgets/qapplication.h>
#include "Exception.h"
#include "../VIMacro.h"
// Provide custom Applications to override notify for logging unhandled exceptions
// Notice : This so-called 'handle' only writes exception information to the log, 
//   does not perform any recovery operations, and the program will not continue to run.
// Because C++ uses memory too freely, unless all developers strictly follow RAII or 
//   use smart pointers, but this is impossible in practice, so it is impossible to ensure 
//   that the program can continue to run safely after an exception occurs.


namespace Visindigo::General {
	class VisindigoAPI CoreApplication :public QCoreApplication {
	public:
		CoreApplication(int& argc, char** argv);
		virtual ~CoreApplication() {}
		virtual bool notify(QObject* receiver, QEvent* event) override;
	};

	class VisindigoAPI GuiApplication :public QGuiApplication {
	public:
		GuiApplication(int& argc, char** argv);
		virtual ~GuiApplication() {}
		virtual bool notify(QObject* receiver, QEvent* event) override;
	};

	class VisindigoAPI Application :public QApplication {
	public:
		Application(int& argc, char** argv);
		virtual ~Application() {}
		virtual bool notify(QObject* receiver, QEvent* event) override;
	};

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
		enum EnvKey {
			PluginFolderPath,
			MinimumLoadingTimeMS,
		};
	public:
		static VIApplication* getInstance();
		VIApplication(int& argc, char** argv, AppType appType = WidgetApp);
		~VIApplication();
		void setMainPlugin(Plugin* plugin);
		void setEnvConfig(EnvKey key, const QVariant& value);
		void onException(const Exception& ex);
		void setLoadingMessageHandler(ApplicationLoadingMessageHandler* handler);
		void setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler);
		void setGlobalFont(const QString& fontPath, int fontID = 0);
		int start();
		QVariant getEnvConfig(EnvKey key) const;
		bool applicationStarted() const;
	private:
		VIApplicationPrivate* d;
	};
}

#define VIApp Visindigo::General::VIApplication::getInstance()