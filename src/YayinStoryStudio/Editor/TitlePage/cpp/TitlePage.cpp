#include <chrono>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qgraphicseffect.h>
#include <QtWidgets/qapplication.h>
#include "../TitlePage.h"

namespace YSS::TitlePage {
	TitlePage::TitlePage() :QFrame() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		// translucent background
		this->setAttribute(Qt::WA_TranslucentBackground);

		Background = new QLabel(this);
		Background->setStyleSheet("QFrame{border-image:url(:/yss/compiled/yayinstorystudio.png);border-radius:10px;}");
		this->setFixedSize(900, 300);
		Background->setGeometry(20, 20, 860, 260);
		ShadowEffect = new QGraphicsDropShadowEffect(this);
		ShadowEffect->setBlurRadius(10); 
		ShadowEffect->setOffset(0, 0);
		ShadowEffect->setColor(QColor(0, 0, 0));
		Background->setGraphicsEffect(ShadowEffect);
		this->setScreen(QApplication::screenAt(QCursor::pos()));

		Title = new QLabel(this);
		Title->setGeometry(20, 230, 860, 50);
		Title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
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