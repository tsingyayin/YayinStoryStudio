#include "../MultiButton.h"

namespace YSSCore::Widgets {
	class MultiButtonPrivate {
		friend class MultiButton;
	protected:
		QString PressedStyleSheet;
		QString HoveredStyleSheet;
		QString NormalStyleSheet;
		bool Pressed = false;
		bool Hovered = false;
	};
	MultiButton::MultiButton(QWidget* parent) :MultiLabel(parent) {
		d = new MultiButtonPrivate;
	}
	MultiButton::~MultiButton() {
		delete d;
	}
	void MultiButton::setPressedStyleSheet(const QString& styleSheet) {
		d->PressedStyleSheet = styleSheet;
	}
	void MultiButton::setHoverStyleSheet(const QString& styleSheet) {
		d->HoveredStyleSheet = styleSheet;
	}
	void MultiButton::setNormalStyleSheet(const QString& styleSheet) {
		d->NormalStyleSheet = styleSheet;
		setStyleSheet(d->NormalStyleSheet);
	}
	bool MultiButton::isHovered() {
		return d->Hovered;
	}
	void MultiButton::mousePressEvent(QMouseEvent* event) {
		MultiLabel::mousePressEvent(event);
		d->Pressed = true;
		setStyleSheet(d->PressedStyleSheet);
		emit pressed();
	}
	void MultiButton::mouseReleaseEvent(QMouseEvent* event) {
		MultiLabel::mouseReleaseEvent(event);
		if (d->Pressed) {
			d->Pressed = false;
			emit clicked();
		}
		emit released();
		setStyleSheet(d->NormalStyleSheet);
	}
	void MultiButton::mouseDoubleClickEvent(QMouseEvent* event) {
		emit doubleClicked();
	}
	void MultiButton::enterEvent(QEnterEvent* event) {
		MultiLabel::enterEvent(event);
		d->Hovered = true;
		setStyleSheet(d->HoveredStyleSheet);
		emit hover();
	}
	void MultiButton::leaveEvent(QEvent* event) {
		MultiLabel::leaveEvent(event);
		d->Hovered = false;
		setStyleSheet(d->NormalStyleSheet);
		emit leave();
	}
}