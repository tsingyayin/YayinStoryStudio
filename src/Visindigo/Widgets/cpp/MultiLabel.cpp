#include "../MultiLabel.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qgridlayout.h>

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
	};

	/*!
		\class Visindigo::Widgets::MultiLabel
		\brief 一个包含标题、描述和图标的标签组件。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		这个类提供一个现代UI中特别常见的需求，即图标-标题-描述的组合展示。
		它允许开发者轻松地创建一个包含图标、标题和描述的标签组件。如果有需要，
		也可以通过addCustomWidget()方法添加一个自定义的子组件，以满足更复杂的布局需求。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	MultiLabel::MultiLabel(QWidget* parent) :QFrame(parent) {
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
		设置标题文本
	*/
	void MultiLabel::setTitle(const QString& str) {
		d->Title->setText(str);
		d->Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	/*!
		\since Visindigo 0.13.0
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
	}

	/*!
		\since Visindigo 0.13.0
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
		设置内容边距
	*/
	void MultiLabel::setContentsMargins(int left, int top, int right, int bottom) {
		d->Layout->setContentsMargins(left, top, right, bottom);
	}

	/*!
		\since Visindigo 0.13.0
		设置组件间距
	*/
	void MultiLabel::setSpacing(int spacing) {
		d->Layout->setSpacing(spacing);
	}

	/*!
		\since Visindigo 0.13.0
		获取标题文本
	*/
	QString MultiLabel::getTitle() {
		return d->Title->text();
	}

	/*!
		\since Visindigo 0.13.0
		获取描述文本
	*/
	QString MultiLabel::getDescription() {
		return d->Description->text();
	}

	/*!
		\since Visindigo 0.13.0
		获取图标路径
	*/
	QString MultiLabel::getPixmapPath() {
		return d->PixmapPath;
	}

	/*!
		\since Visindigo 0.13.0
		设置文本对齐方式
	*/
	void MultiLabel::setAlignment(Qt::Alignment align) {
		d->Title->setAlignment(align);
		d->Description->setAlignment(align);
	}

	/*!
		\since Visindigo 0.13.0
		设置图标固定宽度（高度会自动调整以保持正方形）
	*/
	void MultiLabel::setPixmapFixedWidth(int width) {
		d->Icon->setFixedWidth(width);
		d->Icon->setFixedHeight(width);
	}

	/*!
		\since Visindigo 0.13.0
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
		d->Layout->addWidget(widget, 2, 1, 1, 1);
		d->CustomWidget->show();
	}

	/*!
		\since Visindigo 0.13.0
		重置组件大小时保持图标为正方形
	*/
	void MultiLabel::resizeEvent(QResizeEvent* event) {
		if (d->Icon->isVisible()) {
			d->Icon->setFixedHeight(d->Icon->width());
		}
	}

	/*!
		\since Visindigo 0.13.0
		获取当前的自定义子组件，如果没有则返回nullptr。
	*/
	QWidget* MultiLabel::getCustomWidget() {
		return d->CustomWidget;
	}
}