#ifndef YayinStoryStudio_Editor_TitlePage_TitlePage_h
#define YayinStoryStudio_Editor_TitlePage_TitlePage_h
#include <QtCore/QObject>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <General/VIApplication.h>
class QGraphicsDropShadowEffect;
namespace YSS::TitlePage {
	class TitlePage :public QFrame, public Visindigo::General::ApplicationLoadingMessageHandler {
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
#endif // YayinStoryStudio_Editor_TitlePage_TitlePage_h
