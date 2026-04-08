#include "YSS/ImageViewer.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qscrollbar.h>
namespace YSS::ImageViewer {
	class ImageViewerPrivate {
		friend class ImageViewer;
	protected:
		QLabel* ImageLabel;
		bool CtrlPressed = false;
		QPoint LastMousePos;
		QPixmap Content;
	};

	ImageViewer::ImageViewer(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		d = new ImageViewerPrivate();
		d->ImageLabel = new QLabel(this);
		d->ImageLabel->setAlignment(Qt::AlignCenter);
		d->ImageLabel->resize(this->size());
	}

	ImageViewer::~ImageViewer() {
		delete d;
	}

	bool ImageViewer::onOpen(const QString& filePath) {
		d->Content = QPixmap(filePath);
		if (d->Content.isNull()) {
			return false;
		}
		d->ImageLabel->setPixmap(d->Content);
		d->ImageLabel->resize(d->Content.size());
		d->ImageLabel->move((this->width() - d->Content.width()) / 2, (this->height() - d->Content.height()) / 2);
		emit imageSizeObtained(d->Content.size());
		return true;
	}

	bool ImageViewer::onClose() {
		d->ImageLabel->clear();
		return true;
	}

	bool ImageViewer::onSave(const QString& filePath) {
		return true;
	}

	void ImageViewer::showEvent(QShowEvent* event) {
		this->setFocus();
	}
	void ImageViewer::wheelEvent(QWheelEvent* event) {
		qDebug() << "Wheel Event: " << event->angleDelta();
		if (d->CtrlPressed) {
			d->ImageLabel->resize(d->ImageLabel->size() + QSize(event->angleDelta().y(), event->angleDelta().y()));
			d->ImageLabel->move((this->width() - d->ImageLabel->width()) / 2, (this->height() - d->ImageLabel->height()) / 2);
			d->ImageLabel->setPixmap(d->Content.scaled(d->ImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else {
			d->ImageLabel->move(d->ImageLabel->pos() + QPoint(event->angleDelta().x(), event->angleDelta().y()));
		}
	}

	void ImageViewer::keyPressEvent(QKeyEvent* event) {
		qDebug() << "Key Pressed: " << event->key();
		if (event->modifiers() & Qt::ControlModifier) {
			d->CtrlPressed = true;
		}
		
	}

	void ImageViewer::keyReleaseEvent(QKeyEvent* event) {
		if (event->modifiers() & Qt::ControlModifier) {
			d->CtrlPressed = false;
		}
	}

	void ImageViewer::mousePressEvent(QMouseEvent* event) {
		d->LastMousePos = event->pos();
	}

	void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
		if (event->buttons() & Qt::LeftButton) {
			QPoint delta = event->pos() - d->LastMousePos;
			d->ImageLabel->move(d->ImageLabel->pos() + delta);
			d->LastMousePos = event->pos();
		}
	}

	void ImageViewer::mouseReleaseEvent(QMouseEvent* event) {
	
	}
}