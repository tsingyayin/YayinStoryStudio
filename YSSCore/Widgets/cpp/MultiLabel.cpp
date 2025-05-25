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
		QGridLayout* Layout;
	};
	MultiLabel::MultiLabel(QWidget* parent) :QFrame(parent) {
		d = new MultiLabelPrivate;
		d->Title = new QLabel(this);
		d->Description = new QLabel(this);
		d->Icon = new QLabel(this);
		d->Layout = new QGridLayout(this);
		this->setLayout(d->Layout);
		d->Layout->addWidget(d->Icon, 0, 0, 1, 2);
		d->Layout->addWidget(d->Title, 0, 1, 1, 1);
		d->Layout->addWidget(d->Description, 1, 1, 1, 1);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		this->setPixmapFixedWidth(32);
	}
	MultiLabel::~MultiLabel() {
		delete d;
	}
	void MultiLabel::setTitle(const QString& str) {
		d->Title->setText(str);
	}
	void MultiLabel::setDescription(const QString& str) {
		d->Description->setText(str);
	}
	void MultiLabel::setPixmap(const QPixmap& pixmap) {
		d->Icon->setPixmap(pixmap);
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
	QPixmap MultiLabel::getPixmap() {
		return d->Icon->pixmap();
	}
	void MultiLabel::setAlignment(Qt::Alignment align) {
		d->Title->setAlignment(align);
		d->Description->setAlignment(align);
	}
	void MultiLabel::setPixmapFixedWidth(int width) {
		d->Icon->setFixedWidth(width);
	}
}