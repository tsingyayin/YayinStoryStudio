#include "../HoverTip.h"

namespace YSS::Editor {
	HoverTip::HoverTip(QMouseEvent* event) : QWidget() {
		ShowPos = event->globalPosition();
		this->setAttribute(Qt::WA_TransparentForMouseEvents);
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
		
		ImageLabel = new QLabel(this);
		ImageLabel->setScaledContents(true);
		
		TextBrowser = new QTextBrowser(this);
		TextBrowser->setOpenExternalLinks(true);
		
	}
	void HoverTip::setImage(const QString& path) {
		QPixmap pixmap(path);
		ImageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
	}
	void HoverTip::setMarkdown(const QString& text) {
		TextBrowser->setMarkdown(text);
	}
	void HoverTip::show() {
		this->setGeometry(ShowPos.x(), ShowPos.y(), 200, 200);
		this->show();
	}
}