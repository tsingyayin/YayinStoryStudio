#include "Widgets/TitleWidget.h"
#include "Widgets/private/TitleWidget_p.h"
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include "Utility/ColorTool.h"
#include "General/Log.h"
Visindigo::General::LoggerMsgHandler& operator<<(Visindigo::General::LoggerMsgHandler& handler, const QSize& pos) {
	return handler << "QSize(" + QString::number(pos.width()) + ", " + QString::number(pos.height()) + ")";
}
Visindigo::General::LoggerMsgHandler& operator<<(Visindigo::General::LoggerMsgHandler& handler, const QPoint& pos) {
	return handler << "QPoint(" + QString::number(pos.x()) + ", " + QString::number(pos.y()) + ")";
}
Visindigo::General::LoggerMsgHandler& operator<<(Visindigo::General::LoggerMsgHandler& handler, const QColor& color) {
	return handler << "QColor(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ", " + QString::number(color.alpha()) + ")";
}

namespace Visindigo::__Private__ {
	void TitleWidgetPrivate::setupTopWidget() {
		if (TopWidget) {
			TopWidget->setWindowFlags(Qt::FramelessWindowHint);
		}
	};
	
	void TitleWidgetPrivate::adjustElements() {
		if (Layout) {
			Layout->deleteLater();
		}
		Layout = new QHBoxLayout(q);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(5);
		if (ButtonStyle.testAnyFlag(Visindigo::Widgets::TitleWidget::ButtonRight)) {
			Layout->addWidget(TextLabel);
			if (InsertWidget) {
				Layout->addWidget(InsertWidget);
			}
			Layout->addStretch();
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Minimumize)) {
				Layout->addWidget(MinButton);
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Maximize)) {
				Layout->addWidget(MaxButton);
			}
			if (ButtonFeature.testFlag(Visindigo::Widgets::TitleWidget::Close)) {
				Layout->addWidget(CloseButton);
			}
		
		}else if (ButtonStyle.testAnyFlag(Visindigo::Widgets::TitleWidget::ButtonLeft)) {
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Close)) {
				Layout->addWidget(CloseButton);
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Minimumize)) {
				Layout->addWidget(MinButton);
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Maximize)) {
				Layout->addWidget(MaxButton);
			}
			if (InsertWidget) {
				Layout->addWidget(InsertWidget);
			}
			Layout->addStretch();
			Layout->addWidget(TextLabel);
		}
		q->setLayout(Layout);
	}

	bool TitleWidgetPrivate::eventFilter(QObject* target, QEvent* event) {
		if (target != q && target != TextLabel) {
			return QObject::eventFilter(target, event);
		}
		switch (event->type()) {
		case QEvent::Resize:
			if (target == q) {
				//vgDebug << "TitleWidget resized to:" << q->size();
				MinButton->setFixedSize(q->height() - 4, q->height() - 4);
				MaxButton->setFixedSize(q->height() - 4, q->height() - 4);
				CloseButton->setFixedSize(q->height() - 4, q->height() - 4);
				TextLabel->setFixedHeight(q->height() - 4);
			}
			break;
		case QEvent::MouseButtonPress:
			{
				if (target != TextLabel) {
					break;
				}
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				if (mouseEvent->button() == Qt::LeftButton) {
					LastMousePos = mouseEvent->globalPos() - q->pos();
				}
			}
			break;
		case QEvent::MouseMove:
			if (DragMoveEnable && target == TextLabel){
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				if (mouseEvent->buttons() & Qt::LeftButton) {
					int deltaX = mouseEvent->globalX() - LastMousePos.x() - q->pos().x();
					int deltaY = mouseEvent->globalY() - LastMousePos.y() - q->pos().y();
					TopWidget->move(TopWidget->x() + deltaX, TopWidget->y() + deltaY);
				}
				LastMousePos = mouseEvent->globalPos() - q->pos();
			}
			break;
		}
		return false; // always return false to continue processing the event
	}

	TitleWidget_MinButton::TitleWidget_MinButton(QWidget* parent) :QPushButton(parent) {
		// nothing to do
	}

	TitleWidget_MaxButton::TitleWidget_MaxButton(QWidget* parent) :QPushButton(parent) {
		// nothing to do
	}

	TitleWidget_CloseButton::TitleWidget_CloseButton(QWidget* parent) :QPushButton(parent) {
		// nothing to do
	}

	void TitleWidget_MinButton::paintEvent(QPaintEvent* event) {
		QPushButton::paintEvent(event);
		if (useSign) {
			QSize size = this->size();
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);
			QColor btnColor = this->palette().color(QPalette::Button);
			bool isLight = Utility::ColorTool::isLightColor(btnColor);
			QPen pen;
			pen.setColor(isLight ? Qt::black : Qt::white);
			pen.setWidth(1);
			painter.save();
			painter.setPen(pen);
			// draw short line for minimize button
			painter.drawLine(size.width()*2/10, size.height() / 2 + 1, size.width()*8/10, size.height() / 2 + 1);
			painter.restore();
		}
	}

	void TitleWidget_MaxButton::paintEvent(QPaintEvent* event) {
		QPushButton::paintEvent(event);
		if (useSign) {
			QSize size = this->size();
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);
			QColor btnColor = this->palette().color(QPalette::Button);
			bool isLight = Utility::ColorTool::isLightColor(btnColor);
			QPen pen;
			pen.setColor(isLight ? Qt::black : Qt::white);
			pen.setWidth(1);
			painter.save();
			painter.setPen(pen);
			if (parentWidget()->isMaximized()) {
				// draw two overlapping squares for maximize button
				painter.drawRect(size.width()*2/10, size.height()*4/10,  size.width()*6/10, size.height()*8/10);
				painter.drawRect(size.width() * 4 / 10, size.height() * 2 / 10 , size.width() * 7 / 10, size.height() * 7 / 10);
			}
			else {
				// draw one square for maximize button
				painter.drawRect(size.width()*2/10, size.height()*0.33, size.width()*7/10, size.height()*0.66);
			}
			painter.restore();
		}
	}

	void TitleWidget_CloseButton::paintEvent(QPaintEvent* event) {
		QPushButton::paintEvent(event);
		if (useSign) {
			QSize size = this->size();
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);
			QColor btnColor = this->palette().color(QPalette::Button);
			bool isLight = Utility::ColorTool::isLightColor(btnColor);
			QPen pen;
			pen.setColor(isLight ? Qt::black : Qt::white);
			pen.setWidth(1);
			painter.save();
			painter.setPen(pen);
			// draw X for close button
			painter.drawLine(size.width()*2/10,size.height()*2/10, size.width()*8/10, size.height()*8/10);
			painter.drawLine(size.width()*8/10, size.height()*2/10, size.width()*2/10, size.height()*8/10);
			painter.restore();
		}
	}
}
namespace Visindigo::Widgets {
	TitleWidget::TitleWidget(QWidget* parent, QWidget* topWidget, bool autoSetupTopWidget, bool enableDragMove) :QFrame(parent){
		d = new __Private__::TitleWidgetPrivate();
		d->q = this;
		if (parent && !topWidget) {
			d->TopWidget = parent;
		}else if (topWidget){
			d->TopWidget = topWidget;
		}
		d->MinButton = new Visindigo::__Private__::TitleWidget_MinButton(this);
		d->MinButton->setObjectName("MinButton");
		d->MaxButton = new Visindigo::__Private__::TitleWidget_MaxButton(this);
		d->MaxButton->setObjectName("MaxButton");
		d->CloseButton = new Visindigo::__Private__::TitleWidget_CloseButton(this);
		d->CloseButton->setObjectName("CloseButton");
		d->TextLabel = new QLabel(this);
		d->DragMoveEnable = enableDragMove;
		this->installEventFilter(d);
		d->TextLabel->installEventFilter(d);
		if (autoSetupTopWidget && d->TopWidget) {
			d->setupTopWidget();
		}
		this->setMinimumSize(100, 30);
		d->adjustElements();
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	}

	TitleWidget::~TitleWidget() {
		delete d;
	}

	void TitleWidget::setTopWidget(QWidget* topWidget) {
		d->TopWidget = topWidget;
	}

	void TitleWidget::setTitleText(const QString& text) {
		d->TextLabel->setText(text);
	}

	void TitleWidget::setTitleButtonFeature(TitleButtonFeature feature) {
		d->ButtonFeature = feature;
	}

	void TitleWidget::setButtonStyle(ButtonStyleFeature style) {
		d->ButtonStyle = style;
		if (style.testAnyFlag(SignButton)) {
			d->MinButton->useSign = true;
			d->MaxButton->useSign = true;
			d->CloseButton->useSign = true;
		}
		else {
			d->MinButton->useSign = false;
			d->MaxButton->useSign = false;
			d->CloseButton->useSign = false;
		}
		d->adjustElements();
		update();
	}

	TitleWidget::ButtonStyleFeature TitleWidget::getButtonStyle() const {
		return d->ButtonStyle;
	}

	void TitleWidget::setupTopWidget() {
		d->setupTopWidget();
	}

	void TitleWidget::setDragMoveEnable(bool enable) {
		d->DragMoveEnable = enable;
	}

	void TitleWidget::setInsertWidget(QWidget* widget) {
		d->InsertWidget = widget;
		d->adjustElements();
	}

	void TitleWidget::setTitleAlignment(Qt::Alignment alignment) {
		d->TextLabel->setAlignment(alignment);
	}

	bool TitleWidget::isDragMoveEnable() const {
		return d->DragMoveEnable;
	}
}