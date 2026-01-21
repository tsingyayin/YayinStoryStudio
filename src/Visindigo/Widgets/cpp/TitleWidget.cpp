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
			if (!PixmapPath.isEmpty()) {
				PixmapLabel->show();
				Layout->addWidget(PixmapLabel);
			}
			else {
				PixmapLabel->hide();
			}
			Layout->addWidget(TextLabel);
			if (InsertWidget) {
				Layout->addWidget(InsertWidget);
			}
			Layout->addStretch();
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Minimumize)) {
				MinButton->show();
				Layout->addWidget(MinButton);
			}
			else {
				MinButton->hide();
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Maximize)) {
				MaxButton->show();
				Layout->addWidget(MaxButton);
			}
			else {
				MaxButton->hide();
			}
			if (ButtonFeature.testFlag(Visindigo::Widgets::TitleWidget::Close)) {
				CloseButton->show();
				Layout->addWidget(CloseButton);
			}
			else {
				CloseButton->hide();
			}
		
		}else if (ButtonStyle.testAnyFlag(Visindigo::Widgets::TitleWidget::ButtonLeft)) {
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Close)) {
				CloseButton->show();
				Layout->addWidget(CloseButton);
			}
			else {
				CloseButton->hide();
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Minimumize)) {
				MinButton->show();
				Layout->addWidget(MinButton);
			}
			else {
				MinButton->hide();
			}
			if (ButtonFeature.testAnyFlag(Visindigo::Widgets::TitleWidget::Maximize)) {
				MaxButton->show();
				Layout->addWidget(MaxButton);
			}
			else {
				MaxButton->hide();
			}
			if (InsertWidget) {
				Layout->addWidget(InsertWidget);
			}
			Layout->addStretch();
			Layout->addWidget(TextLabel);
			if (!PixmapPath.isEmpty()) {
				PixmapLabel->show();
				Layout->addWidget(PixmapLabel);
			}
			else {
				PixmapLabel->hide();
			}
		}
		q->setLayout(Layout);
	}

	bool TitleWidgetPrivate::eventFilter(QObject* target, QEvent* event) {
		if (target != q && target != TextLabel && target != TopWidget){
			return QObject::eventFilter(target, event);
		}
		switch (event->type()) {
		case QEvent::WindowStateChange:
			if (target == TopWidget) {
				if (TopWidget->isMaximized()!=MaxButton->isMaximized) {
					MaxButton->isMaximized = TopWidget->isMaximized();
					MaxButton->update();
				}
				qDebug() << "ef" << TopWidget->windowState();
			}
			break;
		case QEvent::Resize:
			if (target == q) {
				//vgDebug << "TitleWidget resized to:" << q->size();
				PixmapLabel->setFixedSize(q->height() - 4, q->height() - 4);
				if (!PixmapPath.isEmpty()) {
					QPixmap pixmap(PixmapPath);
					PixmapLabel->setPixmap(pixmap.scaled(PixmapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
				}
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
			if (TopWidget->isMaximized() || TopWidget->isFullScreen()) {
				break;
			}
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
			if (isMaximized) {
				// draw two overlapping squares for maximize button
				painter.drawRect(size.width()*2/10, size.height()*4/10,  size.width()*4/10, size.height()*3/10);
				painter.drawRect(size.width() * 4 / 10, size.height() * 2 / 10 , size.width() * 4/ 10, size.height() * 3 / 10);
			}
			else {
				// draw one square for maximize button
				painter.drawRect(size.width()*2/10, size.height()*3/10, size.width()*6/10, size.height()*4/10);
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
	/*!
		\class Visindigo::Widgets::TitleWidget
		\inheaderfile Widgets/TitleWidget.h
		\brief 提供一个自定义标题栏控件，可以用于无边框窗口的标题栏显示和操作。
		\since Visindigo 0.13.0
		\inmodule Visindigo
		
		TitleWidget 类继承自 QFrame，提供了一个可定制的标题栏控件。它允许用户设置标题文本、图标、按钮样式等属性，
		并支持拖动窗口移动等功能。该控件适用于需要自定义窗口外观的应用程序，特别是无边框窗口。

		这个类主要在内部依赖事件过滤器实现功能，因此使用时只需要正确使用构造函数就可以自动完成对父控件的劫持，
		不需要额外手动设置。

		这个类当然也支持你正常使用平台本身的窗口，只需要在构造时将后两个布尔量都设置为 false 即可。

		使用无边框窗口时会遇到一个Qt的已知问题，即在无边框窗口最大化后，不能正常恢复到原始大小。
		此类用了一个很蠢的方法来解决这个问题：在恢复大小时，先取消无边框状态，恢复大小后再重新设置无边框状态。
		这会导致窗口在恢复大小时会有一瞬间的边框闪现以及消失，但这是目前唯一简单有效的解决方法。

		这个类允许你通过枚举值 TitleWidget::TitleButtonFlag 和 TitleWidget::ButtonStyleFlag 来定制标题栏按钮的功能和位置。
		基本上就是选择显示哪些按钮，使用符号化风格还是圆点风格（类似MacOS），以及按钮是放在标题栏的左侧还是右侧。

		此外，它还允许你在其中插入自定义的 QWidget 作为标题栏的一部分，从而实现更复杂的布局需求。这在
		现代应用程序中很常见，例如在标题栏中添加搜索框、菜单按钮等。
	*/

	/*!
		\enum Visindigo::Widgets::TitleWidget::TitleButtonFlag
		\since Visindigo 0.13.0
		\value Minimumize 指定显示最小化按钮。
		\value Maximize 指定显示最大化/还原按钮。
		\value Close 指定显示关闭按钮。
		\value NotMinimumize 指定不显示最小化按钮。
		\value All 指定显示所有按钮（最小化、最大化/还原、关闭）。
	*/

	/*!
		\enum Visindigo::Widgets::TitleWidget::ButtonStyleFlag
		\since Visindigo 0.13.0
		\value SignButton 指定使用符号化风格的按钮（类似Windows风格）。
		\value RoundButton 指定使用圆点风格的按钮（类似MacOS风格）。
		\value ButtonLeft 指定将按钮放置在标题栏的左侧。
		\value ButtonRight 指定将按钮放置在标题栏的右侧。
		\value Win32Style 指定使用符号化风格的按钮并将其放置在右侧。
		\value MacOSStyle 指定使用圆点风格的按钮并将其放置在左侧。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 指定 TitleWidget 的父控件。如果提供了该参数且 \a topWidget 为空，
		则 TitleWidget 会自动将父控件作为其管理的顶层窗口。
		\a topWidget 指定 TitleWidget 所管理的顶层窗口。如果提供了该参数，
		则 TitleWidget 会将其作为顶层窗口进行管理。
		\a autoSetupTopWidget 指定是否自动将 \a parent 或 \a topWidget 设置为无边框窗口。
		\a enableDragMove 指定是否启用通过拖动标题栏移动窗口的功能。

		TitleWidget 构造函数。
	*/
	TitleWidget::TitleWidget(QWidget* parent, QWidget* topWidget, bool autoSetupTopWidget, bool enableDragMove) :QFrame(parent){
		d = new __Private__::TitleWidgetPrivate();
		d->q = this;
		if (parent && !topWidget) {
			d->TopWidget = parent;
		}else if (topWidget){
			d->TopWidget = topWidget;
			
		}
		d->TopWidget->installEventFilter(d);
		d->MinButton = new Visindigo::__Private__::TitleWidget_MinButton(this);
		d->MinButton->setObjectName("MinButton");
		d->MaxButton = new Visindigo::__Private__::TitleWidget_MaxButton(this);
		d->MaxButton->setObjectName("MaxButton");
		d->CloseButton = new Visindigo::__Private__::TitleWidget_CloseButton(this);
		d->CloseButton->setObjectName("CloseButton");
		d->PixmapLabel = new QLabel(this);
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
		connect(d->MinButton, &QPushButton::clicked, [this]() {
			if (d->TopWidget) {
				d->TopWidget->showMinimized();
			}
			});
		connect(d->MaxButton, &QPushButton::clicked, [this]() {
			if (d->TopWidget) {
				qDebug() << "bt1"<<d->TopWidget->windowState();
				if (d->TopWidget->isMaximized()) {
					bool frameless = d->TopWidget->windowFlags().testAnyFlag(Qt::FramelessWindowHint);
					QRect restoreRect = d->TopWidget->normalGeometry();
					if (frameless) {
						d->TopWidget->setWindowFlags(d->TopWidget->windowFlags() & ~Qt::FramelessWindowHint);
						d->TopWidget->show();
					}
					d->TopWidget->showNormal();
					if (frameless) {
						d->TopWidget->setWindowFlags(d->TopWidget->windowFlags() | Qt::FramelessWindowHint);
						d->TopWidget->setGeometry(restoreRect);
						d->TopWidget->show();
					}
				}
				else {
					d->TopWidget->showMaximized();
				}
				qDebug() << "bt2"<<d->TopWidget->windowState();
				d->MaxButton->isMaximized = d->TopWidget->isMaximized();
				d->MaxButton->update();
			}
			});
		connect(d->CloseButton, &QPushButton::clicked, [this]() {
			if (d->TopWidget) {
				d->TopWidget->close();
			}
			});
	}

	/*!
		\since Visindigo 0.13.0
		TitleWidget 析构函数。
	*/
	TitleWidget::~TitleWidget() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置 TitleWidget 所管理的顶层窗口为 \a topWidget。
	*/
	void TitleWidget::setTopWidget(QWidget* topWidget) {
		d->TopWidget = topWidget;
	}

	/*!
		\since Visindigo 0.13.0
		返回 TitleWidget 所管理的顶层窗口。
	*/
	QWidget* TitleWidget::getTopWidget() const {
		return d->TopWidget;
	}

	/*!
		\since Visindigo 0.13.0
		设置标题文本为 \a text。
	*/
	void TitleWidget::setTitleText(const QString& text) {
		d->TextLabel->setText(text);
	}

	/*!
		\since Visindigo 0.13.0
		返回当前的标题文本。
	*/
	QString TitleWidget::getTitleText() const {
		return d->TextLabel->text();
	}

	/*!
		\since Visindigo 0.13.0
		设置标题图标的文件路径为 \a filePath。
	*/
	void TitleWidget::setPixmapPath(const QString& filePath) {
		d->PixmapPath = filePath;
		QPixmap pixmap(filePath);
		d->PixmapLabel->setPixmap(pixmap.scaled(d->PixmapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		d->adjustElements();
		update();
	}

	/*!
		\since Visindigo 0.13.0
		返回当前标题图标的文件路径。
	*/
	QString TitleWidget::getPixmapPath() const {
		return d->PixmapPath;
	}

	/*!
		\since Visindigo 0.13.0
		设置标题栏按钮的功能为 \a feature。
	*/
	void TitleWidget::setTitleButtonFeature(TitleButtonFeature feature) {
		d->ButtonFeature = feature;
	}

	/*!
		\since Visindigo 0.13.0
		返回当前标题栏按钮的功能。
	*/
	TitleWidget::TitleButtonFeature TitleWidget::getTitleButtonFeature() const {
		return d->ButtonFeature;
	}

	/*!
		\since Visindigo 0.13.0
		设置标题栏按钮的样式为 \a style。
	*/
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

	/*!
		\since Visindigo 0.13.0
		返回当前标题栏按钮的样式。
	*/
	TitleWidget::ButtonStyleFeature TitleWidget::getButtonStyle() const {
		return d->ButtonStyle;
	}

	/*!
		\since Visindigo 0.13.0
		设置 TitleWidget 所管理的顶层窗口为无边框窗口。
	*/
	void TitleWidget::setupTopWidget() {
		d->setupTopWidget();
	}

	/*!
		\since Visindigo 0.13.0
		设置是否启用通过拖动标题栏移动窗口的功能为 \a enable。
	*/
	void TitleWidget::setDragMoveEnable(bool enable) {
		d->DragMoveEnable = enable;
	}

	/*!
		\since Visindigo 0.13.0
		设置插入控件为 \a widget。
	*/
	void TitleWidget::setInsertWidget(QWidget* widget) {
		d->InsertWidget = widget;
		d->adjustElements();
	}

	/*!
		\since Visindigo 0.13.0
		返回当前的插入控件。
	*/
	QWidget* TitleWidget::getInsertWidget() const {
		return d->InsertWidget;
	}

	/*!
		\since Visindigo 0.13.0
		设置标题文本的对齐方式为 \a alignment。
	*/
	void TitleWidget::setTitleAlignment(Qt::Alignment alignment) {
		d->TextLabel->setAlignment(alignment);
	}

	/*!
		\since Visindigo 0.13.0
		返回当前标题文本的对齐方式。
	*/
	Qt::Alignment TitleWidget::getTitleAlignment() const {
		return d->TextLabel->alignment();
	}

	/*!
		\since Visindigo 0.13.0
		返回是否启用通过拖动标题栏移动窗口的功能。
	*/
	bool TitleWidget::isDragMoveEnable() const {
		return d->DragMoveEnable;
	}
}