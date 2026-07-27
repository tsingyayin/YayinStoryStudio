#ifndef YayinStoryStudio_Editor_SimpleWidget_EAWarningWidget_h
#define YayinStoryStudio_Editor_SimpleWidget_EAWarningWidget_h
#include <QtWidgets/qwidget.h>

class QLabel;
class QPushButton;
class QVBoxLayout;
namespace YSS::SimpleWidget {
	class EAWarningWidget :public QWidget
	{
		Q_OBJECT;
	private:
		QLabel* Title;
		QLabel* Description;
		QPushButton* OkButton;
		QVBoxLayout* Layout;
	public:
		EAWarningWidget();
		virtual void closeEvent(QCloseEvent* event) override;
	};
}
#endif // YayinStoryStudio_Editor_SimpleWidget_EAWarningWidget_h
