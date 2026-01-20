#ifndef VISINDIGO_GENERAL_VIAPPLICATION_P_H
#define VISINDIGO_GENERAL_VIAPPLICATION_P_H
#include "../VIApplication.h"

namespace Visindigo::__Private__ {
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

	class ApplicationLoadingMessageHandlerDefaultConsoleImpl : public General::ApplicationLoadingMessageHandler {
		virtual void onLoadingMessage(const QString& msg) override;
		virtual void enableHandler() override;
		virtual void disableHandler() override;
	};
}
#endif