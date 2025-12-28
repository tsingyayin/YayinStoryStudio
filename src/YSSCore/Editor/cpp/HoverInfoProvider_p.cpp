#include "Editor/private/HoverInfoProvider_p.h"
#include <QtWidgets/qscrollbar.h>
#include <Widgets/ThemeManager.h>

namespace YSSCore::__Private__ {
	HoverInfoWidget::HoverInfoWidget(QWidget* parent) :QFrame(parent) {
		this->setMinimumSize(400, 80);
		ContentArea = new QTextBrowser(this);
		Layout = new QVBoxLayout(this);
		this->setLayout(Layout);
		Layout->addWidget(ContentArea);
		this->setStyleSheet(YSSTMSS("YSS::Default"));
		this->ContentArea->verticalScrollBar()->setStyleSheet(YSSTMSS("YSS::NormalScrollBar", this));
	}

	void HoverInfoWidget::setPlainText(const QString& text) {
		ContentArea->setPlainText(text);
	}

	void HoverInfoWidget::setMarkdown(const QString& md) {
		ContentArea->setMarkdown(md);
	}

	void HoverInfoWidget::setHtml(const QString& html) {
		ContentArea->setHtml(html);
	}

	void HoverInfoWidget::scrollBy(qint32 deltaY) {
		QScrollBar* vBar = ContentArea->verticalScrollBar();
		vBar->setValue(vBar->value() + deltaY);
	}
}