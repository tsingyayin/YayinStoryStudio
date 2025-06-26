#include "../EAWarningWidget.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>

namespace YSS::SimpleWidget {
	EAWarningWidget::EAWarningWidget() :QWidget() {
		Title = new QLabel(this);
		Description = new QLabel(this);
		OkButton = new QPushButton(this);
		Layout = new QVBoxLayout(this);

		this->setLayout(Layout);
		Layout->addWidget(Title);
		Layout->addWidget(Description);
		Layout->addWidget(OkButton);

		Description->setWordWrap(true);
	}
	void EAWarningWidget::closeEvent(QCloseEvent* event) {
	}
}