#include "../MultiButton.h"
#include "../private/MultiButton_p.h"
#include <QtWidgets/qstyleoption.h>
#include <QtWidgets/qstyle.h>
#include <QtGui/qpainter.h>
#include "General/Log.h"
namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::MultiButton
		\brief MultiLabel的扩展，提供了按钮的功能。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		这个类在Visindigo::Widgets::MultiLabel的基础上添加了按钮的功能。
		它提供了不同状态下的样式表设置，并且可以通过信号槽机制响应用户的点击、悬停等操作。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::clicked()
		\since Visindigo 0.13.0
		当按钮被"咔哒"时发出此信号。即在此按钮区域内按下且释放鼠标时发出此信号。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::doubleClicked()
		\since Visindigo 0.13.0
		当按钮被双击时发出此信号。即在此按钮区域内连续两次按下鼠标时发出此信号。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::pressed()
		\since Visindigo 0.13.0
		当按钮被按下时发出此信号。即在此按钮区域内按下鼠标时发出此信号。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::released()
		\since Visindigo 0.13.0
		当按钮被释放时发出此信号。即在此按钮区域内释放鼠标时发出此信号。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::hover()
		\since Visindigo 0.13.0
		当鼠标悬停在按钮上时发出此信号。即当鼠标进入此按钮区域时发出此信号。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButton::leave()
		\since Visindigo 0.13.0
		当鼠标离开按钮时发出此信号。即当鼠标离开此按钮区域时发出此信号。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	MultiButton::MultiButton(QWidget* parent) :MultiLabel(parent) {
		d = new Visindigo::__Private__::MultiButtonPrivate;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	MultiButton::~MultiButton() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置按钮被按下时的样式表。
	*/
	void MultiButton::setPressedStyleSheet(const QString& styleSheet) {
		d->PressedStyleSheet = styleSheet;
	}

	/*!
		\since Visindigo 0.13.0
		设置按钮被悬停时的样式表。
	*/
	void MultiButton::setHoverStyleSheet(const QString& styleSheet) {
		d->HoveredStyleSheet = styleSheet;
	}

	/*!
		\since Visindigo 0.13.0
		设置按钮的正常状态下的样式表。
	*/
	void MultiButton::setNormalStyleSheet(const QString& styleSheet) {
		d->NormalStyleSheet = styleSheet;
		setStyleSheet(d->NormalStyleSheet);
	}

	/*!
		\since Visindigo 0.13.0
		设置按钮的非活动状态下的样式表。
	*/
	void MultiButton::setInactiveStyleSheet(const QString& styleSheet) {
		d->InactiveStyleSheet = styleSheet;
		if (!d->Active) {
			setStyleSheet(d->InactiveStyleSheet);
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置按钮是否处于活动状态。
	*/
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

	/*!
		\since Visindigo 0.13.0
		获取按钮是否处于活动状态。
	*/
	bool MultiButton::isActive() {
		return d->Active;
	}

	/*!
		\since Visindigo 0.13.0
		获取按钮是否处于悬停状态。
	*/
	bool MultiButton::isHovered() {
		return d->Hovered;
	}

	/*!
		\since Visindigo 0.14.0
		绘制事件处理函数。此函数会在按钮需要重绘时被调用。
	*/
	void MultiButton::paintEvent(QPaintEvent* event) {
		if (not d->inButtonGroup) {
			QStyleOptionButton option;
			option.initFrom(this);
			if (d->Active) {
				if (d->Pressed) {
					option.state |= QStyle::State_Sunken;
				}
				if (d->Hovered) {
					option.state |= QStyle::State_MouseOver;
				}
			}
			QPainter painter(this);
			QStyle* style = this->style();
			style->drawControl(QStyle::CE_PushButton, &option, &painter, this);
		}
		else {
			QStyleOptionViewItem option;
			option.initFrom(this);
			if (d->Active) {
				if (d->Pressed) {
					option.state |= QStyle::State_Sunken;
				}
				if (d->Hovered) {
					option.state |= QStyle::State_MouseOver;
				}
				if (d->buttonGroupChecked) {
					option.state |= QStyle::State_On;
				}
				else {
					option.state |= QStyle::State_Off;
				}
			}
			QPainter painter(this);
			QStyle* style = this->style();
			style->drawControl(QStyle::CE_ItemViewItem, &option, &painter, this);
		}
	}
	/*!
		\since Visindigo 0.13.0
		鼠标按下事件处理函数。此函数会在鼠标按下时被调用。
	*/
	void MultiButton::mousePressEvent(QMouseEvent* event) {
		MultiLabel::mousePressEvent(event);
		if (!d->Active) {
			return;
		}
		d->Pressed = true;
		if(d->PressedStyleSheet.isEmpty()) {
			repaint();
		}
		else {
			setStyleSheet(d->PressedStyleSheet);
		}
		emit pressed();
	}

	/*!
		\since Visindigo 0.13.0
		鼠标释放事件处理函数。此函数会在鼠标释放时被调用。
	*/
	void MultiButton::mouseReleaseEvent(QMouseEvent* event) {
		MultiLabel::mouseReleaseEvent(event);
		if (!d->Active) {
			return;
		}
		if (d->NormalStyleSheet.isEmpty()) {
			repaint();
		}
		else {
			setStyleSheet(d->NormalStyleSheet);
		}
		emit released();
		if (d->Pressed) {
			d->Pressed = false;
			emit clicked();
		}
	}

	/*!
		\since Visindigo 0.13.0
		鼠标双击事件处理函数。此函数会在鼠标双击时被调用。
	*/
	void MultiButton::mouseDoubleClickEvent(QMouseEvent* event) {
		MultiLabel::mouseDoubleClickEvent(event);
		if (!d->Active) {
			return;
		}
		emit doubleClicked();
	}

	/*!
		\since Visindigo 0.13.0
		鼠标进入事件处理函数。此函数会在鼠标进入按钮区域时被调用。
	*/
	void MultiButton::enterEvent(QEnterEvent* event) {
		MultiLabel::enterEvent(event);
		d->Hovered = true;
		if (d->HoveredStyleSheet.isEmpty()) {
			repaint();
		}
		else {
			setStyleSheet(d->HoveredStyleSheet);
		}
		emit hover();
	}

	/*!
		\since Visindigo 0.13.0
		鼠标离开事件处理函数。此函数会在鼠标离开按钮区域时被调用。
	*/
	void MultiButton::leaveEvent(QEvent* event) {
		MultiLabel::leaveEvent(event);
		d->Hovered = false;
		if (d->NormalStyleSheet.isEmpty()) {
			repaint();
		}
		else {
			setStyleSheet(d->NormalStyleSheet);
		}
		emit leave();
	}
}