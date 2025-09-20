#include "../MultiButton.h"
#include "../private/MultiButton_p.h"
namespace Visindigo::Widgets {
	MultiButton::MultiButton(QWidget* parent) :MultiLabel(parent) {
		d = new Visindigo::__Private__::MultiButtonPrivate;
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
	void MultiButton::setInactiveStyleSheet(const QString& styleSheet) {
		d->InactiveStyleSheet = styleSheet;
		if (!d->Active) {
			setStyleSheet(d->InactiveStyleSheet);
		}
	}
	void MultiButton::setActive(bool active) {
		d->Active = active;
		if (!d->Active) {
			if (d->InactiveStyleSheet.isEmpty()) {
				setStyleSheet(d->NormalStyleSheet);
			}
			else {
				setStyleSheet(d->InactiveStyleSheet);
			}
		}
	}
	bool MultiButton::isActive() {
		return d->Active;
	}
	bool MultiButton::isHovered() {
		return d->Hovered;
	}
	void MultiButton::mousePressEvent(QMouseEvent* event) {
		MultiLabel::mousePressEvent(event);
		if (!d->Active) {
			return;
		}
		d->Pressed = true;
		setStyleSheet(d->PressedStyleSheet);
		emit pressed();
	}
	void MultiButton::mouseReleaseEvent(QMouseEvent* event) {
		MultiLabel::mouseReleaseEvent(event);
		if (!d->Active) {
			return;
		}
		setStyleSheet(d->NormalStyleSheet);
		emit released();
		if (d->Pressed) {
			d->Pressed = false;
			emit clicked();
		}
	}
	void MultiButton::mouseDoubleClickEvent(QMouseEvent* event) {
		MultiLabel::mouseDoubleClickEvent(event);
		if (!d->Active) {
			return;
		}
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