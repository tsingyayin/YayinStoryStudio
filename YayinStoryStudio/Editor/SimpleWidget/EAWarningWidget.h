#pragma once
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