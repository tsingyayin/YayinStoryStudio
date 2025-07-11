#include "../MultiLabel.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qgridlayout.h>

namespace YSSCore::Widgets {
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
		d->Layout->addWidget(d->Icon, 0, 0, 2, 1);
		d->Layout->addWidget(d->Title, 0, 1, 1, 1);
		d->Layout->addWidget(d->Description, 1, 1, 1, 1);
		d->Layout->setRowStretch(0, 2);
		d->Layout->setRowStretch(1, 1);
		d->Icon->setAlignment(Qt::AlignCenter);
		d->Title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		d->Description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		this->setPixmapFixedWidth(64);
		d->Icon->hide();
		d->Description->hide();
	}
	MultiLabel::~MultiLabel() {
		delete d;
	}
	void MultiLabel::setTitle(const QString& str) {
		d->Title->setText(str);
	}
	void MultiLabel::setDescription(const QString& str) {
		d->Description->setText(str);
		if (!str.isEmpty()) {
			d->Description->show();
		}
	}
	void MultiLabel::setPixmapPath(const QString& filePath) {
		d->Icon->setStyleSheet("QLabel{border-image: url(" + filePath + ");}");
		d->PixmapPath = filePath;
		d->Icon->setFixedHeight(d->Icon->width());
		d->Icon->show();
	}
	void MultiLabel::setContentsMargins(int left, int top, int right, int bottom) {
		d->Layout->setContentsMargins(left, top, right, bottom);
	}
	void MultiLabel::setSpacing(int spacing) {
		d->Layout->setSpacing(spacing);
	}
	QString MultiLabel::getTitle() {
		return d->Title->text();
	}
	QString MultiLabel::getDescription() {
		return d->Description->text();
	}
	QString MultiLabel::getPixmapPath() {
		return d->PixmapPath;
	}
	void MultiLabel::setAlignment(Qt::Alignment align) {
		d->Title->setAlignment(align);
		d->Description->setAlignment(align);
	}
	void MultiLabel::setPixmapFixedWidth(int width) {
		d->Icon->setFixedWidth(width);
	}
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
	void MultiLabel::resizeEvent(QResizeEvent* event) {
		if (d->Icon->isVisible()) {
			d->Icon->setFixedHeight(d->Icon->width());
		}
	}
	QWidget* MultiLabel::getCustomWidget() {
		return d->CustomWidget;
	}
}