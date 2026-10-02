#ifndef YayinStoryStudio_Widgets_BSOD_h
#define YayinStoryStudio_Widgets_BSOD_h
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollarea.h>
#include <General/VIApplication.h>
namespace YSS::Widgets {
	class BSOD :public QWidget, public Visindigo::General::ApplicationExceptionMessageHandler {
		Q_OBJECT;
	signals:
		void dismissed();
	public:
		BSOD(QWidget* parent = nullptr);
		virtual ~BSOD();
		virtual void onExceptionMessage(const Visindigo::General::Exception& ex) override;
		virtual void enableHandler() override;
		virtual void exec() override;
		virtual void disableHandler() override;
	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
	private:
		QLabel* makeLabel(const QString& text, const QString& color, int pixelSize, bool selectable);
		QString buildStacktrace(const Visindigo::General::Exception& ex) const;
		QVBoxLayout* Layout;
		QLabel* Face;
		QLabel* Headline;
		QLabel* Detail;
		QLabel* Progress;
		QLabel* StopCode;
		QLabel* Location;
		QLabel* TraceCaption;
		QLabel* Trace;
		QScrollArea* TraceArea;
		QLabel* Artifact;
		QPushButton* ConfirmButton;
	};
}
#endif // YayinStoryStudio_Widgets_BSOD_h
