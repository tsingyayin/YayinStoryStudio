#include <QtWidgets/qscrollbar.h>
#include <Widgets/LiquidGlassEffect.h>
#include <Widgets/ThemeManager.h>
#include "Editor/private/HoverInfoProvider_p.h"

namespace YSSCore::__Private__ {
	HoverInfoWidget::HoverInfoWidget(QWidget* parent) :QFrame(parent) {
		this->setFixedSize(DefaultWidth, DefaultHeight);
		auto* glass = new Visindigo::Widgets::LiquidGlassEffect(this);
		glass->setBorderRadius(6);
		glass->setBlurRadius(20);
		this->setGraphicsEffect(glass);
		auto font = this->font();
		font.setPointSizeF(font.pointSizeF() * 0.8);
		this->setFont(font);
		ContentArea = new QTextBrowser(this);
		Layout = new QVBoxLayout(this);
		this->setLayout(Layout);
		Layout->addWidget(ContentArea);
		Layout->setContentsMargins(0, 0, 0, 0);
		Visindigo::Widgets::LiquidGlassEffect::applyForegroundAlpha(this);
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

	void HoverInfoWidget::recoverDefaultWidth() {
		this->setFixedWidth(DefaultWidth);
	}
}