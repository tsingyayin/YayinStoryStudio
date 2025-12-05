#include <chrono>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qapplication.h>
#include "../TitlePage.h"

namespace YSS::TitlePage {
	TitlePage::TitlePage() :QFrame() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		this->setStyleSheet("QFrame{border-image:url(:/yss/compiled/yayinstorystudio.png)}");
		this->setFixedSize(900, 300);
		this->setScreen(QApplication::screenAt(QCursor::pos()));

		Title = new QLabel(this);
		Title->setGeometry(0, 250, 850, 50);
		Title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		Title->setStyleSheet("QLabel{border-image:url();color:#BBBBBB}");
		Title->setText("Loading...");
	}
	void TitlePage::onLoadingMessage(const QString& msg) {
		Title->setText(msg);
		this->repaint();
	}
	void TitlePage::enableHandler() {
		this->show();
	}
	void TitlePage::disableHandler() {
		this->close();
	}
}