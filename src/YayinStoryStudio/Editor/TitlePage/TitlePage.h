#pragma once
#include <QtCore/QObject>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <General/VIApplication.h>
class QGraphicsDropShadowEffect;
namespace YSS::TitlePage {
	class TitlePage :public QFrame , public Visindigo::General::ApplicationLoadingMessageHandler {
		Q_OBJECT;
		QLabel* Background;
		QLabel* Title;
		QGraphicsDropShadowEffect* ShadowEffect;
	public:
		TitlePage();
		virtual void onLoadingMessage(const QString& msg) override;
		virtual void enableHandler() override;
		virtual void disableHandler() override;
	};
}