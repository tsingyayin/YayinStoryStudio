#ifndef VISINDIGO_GENERAL_VIAPPLICATION_P_H
#define VISINDIGO_GENERAL_VIAPPLICATION_P_H
#include "../VIApplication.h"

namespace Visindigo::__Private__ {
	class ApplicationLoadingMessageHandlerDefaultConsoleImpl : public General::ApplicationLoadingMessageHandler {
		virtual void onLoadingMessage(const QString& msg) override;
		virtual void enableHandler() override;
		virtual void disableHandler() override;
	};
}
#endif