#include "../MultiLabel.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qapplication.h>
namespace Visindigo::Widgets {
	class MultiLabelPrivate {
		friend class MultiLabel;
	protected:
		QLabel* Title;
		QLabel* Description;
		QLabel* Icon;
		QString PixmapPath;
		QGridLayout* Layout;
		QWidget* CustomWidget = nullptr;
		Qt::Orientation CustomWidgetOrientation = Qt::Horizontal;

		void adjustLayout() {
			if (CustomWidget) {
				if (CustomWidgetOrientation == Qt::Horizontal) {
					Layout->addWidget(CustomWidget, 0, 2, 2, 1);
				}
				else {
					Layout->addWidget(CustomWidget, 2, 1, 1, 1);
				}
			}
		}
	};

	/*!
		\class Visindigo::Widgets::MultiLabel
		\brief 一个包含标题、描述和图标的标签组件.
		\since Visindigo 0.13.0
		\inmodule Visindigo

		这个类提供一个现代UI中特别常见的需求，即图标-标题-描述的组合展示。
		它允许开发者轻松地创建一个包含图标、标题和描述的标签组件。如果有需要，
		也可以通过addCustomWidget()方法添加一个自定义的子组件，以满足更复杂的布局需求。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 父窗口

		构造函数
	*/
	MultiLabel::MultiLabel(QWidget* parent) : BorderFrame(parent) {
		d = new MultiLabelPrivate;
		d->Title = new QLabel(this);
		d->Title->setObjectName("TitleLabel");
		d->Description = new QLabel(this);
		d->Description->setObjectName("DescriptionLabel");
		d->Description->setWordWrap(true);
		d->Icon = new QLabel(this);
		d->Icon->setObjectName("IconLabel");
		d->Layout = new QGridLayout(this);
		this->setLayout(d->Layout);
		d->Layout->addWidget(d->Title, 0, 1, 1, 1);
		d->Icon->setAlignment(Qt::AlignCenter);
		d->Title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		d->Description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		this->setPixmapFixedWidth(64);
		d->Icon->hide();
		d->Description->hide();
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	MultiLabel::~MultiLabel() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		\a str 要设置的标题文本

		设置标题文本
	*/
	void MultiLabel::setTitle(const QString& str) {
		d->Title->setText(str);
		d->Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	/*!
		\since Visindigo 0.13.0
		\a str 要设置的描述文本

		设置描述文本
	*/
	void MultiLabel::setDescription(const QString& str) {
		d->Description->setText(str);
		if (!str.isEmpty()) {
			d->Layout->addWidget(d->Description, 1, 1, 1, 1);
			d->Layout->setRowStretch(0, 2);
			d->Layout->setRowStretch(1, 1);
			d->Description->show();
		}
		QFont font = qApp->font();
		font.setPointSizeF(font.pointSizeF() * 0.8);
		d->Description->setFont(font);
	}

	/*!
		\since Visindigo 0.13.0
		\a filePath 图标文件路径，可以是相对路径或绝对路径，支持常见的图像格式如PNG、JPEG等。

		设置图标路径
	*/
	void MultiLabel::setPixmapPath(const QString& filePath) {
		if (filePath.isEmpty()) {
			return;
		}
		d->Icon->setStyleSheet("QLabel{border-image: url(" + filePath + ");}");
		d->PixmapPath = filePath;
		d->Layout->addWidget(d->Icon, 0, 0, 2, 1);
		d->Icon->setFixedHeight(d->Icon->width());
		d->Icon->show();
	}

	/*!
		\since Visindigo 0.13.0
		\a left 左边距, \a top 上边距, \a right 右边距, \a bottom 下边距

		设置内容边距
	*/
	void MultiLabel::setContentsMargins(int left, int top, int right, int bottom) {
		d->Layout->setContentsMargins(left, top, right, bottom);
	}

	/*!
		\since Visindigo 0.13.0
		\a spacing 组件间距

		设置组件间距
	*/
	void MultiLabel::setSpacing(int spacing) {
		d->Layout->setSpacing(spacing);
	}

	/*!
		\since Visindigo 0.13.0
		return 标题文本
	*/
	QString MultiLabel::getTitle() {
		return d->Title->text();
	}

	/*!
		\since Visindigo 0.13.0
		return 描述文本
	*/
	QString MultiLabel::getDescription() {
		return d->Description->text();
	}

	/*!
		\since Visindigo 0.13.0
		return 图标路径
	*/
	QString MultiLabel::getPixmapPath() {
		return d->PixmapPath;
	}

	/*!
		\since Visindigo 0.13.0
		\a align 要设置的对齐方式

		设置文本对齐方式
	*/
	void MultiLabel::setAlignment(Qt::Alignment align) {
		d->Title->setAlignment(align);
		d->Description->setAlignment(align);
	}

	/*!
		\since Visindigo 0.13.0
		\a width 图标固定宽度，单位为像素

		设置图标固定宽度（高度会自动调整以保持正方形）
	*/
	void MultiLabel::setPixmapFixedWidth(int width) {
		d->Icon->setFixedWidth(width);
		d->Icon->setFixedHeight(width);
	}

	/*!
		\since Visindigo 0.13.0
		\a widget 要添加的自定义子组件

		添加一个自定义的子组件，位于标题和描述的右侧。

		这个函数会接管设置组件的所有权。调用此函数后
		不应再在外部保留该组件的指针。
	*/
	void MultiLabel::addCustomWidget(QWidget* widget) {
		if (d->CustomWidget != nullptr) {
			d->Layout->removeWidget(d->CustomWidget);
			d->CustomWidget->deleteLater();
		}
		d->CustomWidget = widget;
		widget->setParent(this);
		d->CustomWidget->show();
		d->adjustLayout();
	}

	/*!
		\since Visindigo 0.14.0
		\a orientation 布局方向

		设置自定义子组件的布局方向，默认为水平布局。
	*/
	void MultiLabel::setCustomWidgetOrientation(Qt::Orientation orientation) {
		d->CustomWidgetOrientation = orientation;
		d->adjustLayout();
	}

	/*!
		\since Visindigo 0.13.0
		\a event resize事件
		
		由MultiLabel重载的resizeEvent函数，用于在组件大小发生改变时调整图标的高度以保持正方形。
		如果继承此类并重写了resizeEvent函数，则必须显式调用此函数以保持图标的正确显示，否则图标可能会变形或布局可能会错乱。
	*/
	void MultiLabel::resizeEvent(QResizeEvent* event) {
		if (d->Icon->isVisible()) {
			d->Icon->setFixedHeight(d->Icon->width());
		}
	}

	/*!
		\since Visindigo 0.13.0
		return 当前的自定义子组件，如果没有则返回nullptr。
	*/
	QWidget* MultiLabel::getCustomWidget() {
		return d->CustomWidget;
	}
}