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
		QScrollArea* ScrollArea;
		QHBoxLayout* Layout;
		bool CtrlPressed = false;
		bool AltPressed = false;
		QPoint LastMousePos;
	};

	ImageViewer::ImageViewer(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		d = new ImageViewerPrivate();
		d->ImageLabel = new QLabel(this);
		d->ImageLabel->setAlignment(Qt::AlignCenter);
		d->ScrollArea = new QScrollArea(this);
		d->ScrollArea->setWidget(d->ImageLabel);
		d->Layout = new QHBoxLayout(this);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->ScrollArea);
	}

	ImageViewer::~ImageViewer() {
		delete d;
	}

	bool ImageViewer::onOpen(const QString& filePath) {
		QPixmap pixmap(filePath);
		if (pixmap.isNull()) {
			return false;
		}
		d->ImageLabel->setPixmap(pixmap);
		d->ImageLabel->resize(pixmap.size());
		emit imageSizeObtained(pixmap.size());
		return true;
	}

	bool ImageViewer::onClose() {
		d->ImageLabel->clear();
		return true;
	}

	bool ImageViewer::onSave(const QString& filePath) {
		return true;
	}

	void ImageViewer::wheelEvent(QWheelEvent* event) {
		if (d->CtrlPressed) {
			d->ScrollArea->horizontalScrollBar()->setValue(d->ScrollArea->horizontalScrollBar()->value() - event->angleDelta().y());
		}
		else if (d->AltPressed) {
			d->ImageLabel->resize(d->ImageLabel->size() + QSize(event->angleDelta().y(), event->angleDelta().y()));
		}
		else {
			d->ScrollArea->verticalScrollBar()->setValue(d->ScrollArea->verticalScrollBar()->value() - event->angleDelta().y());
		}
	}

	void ImageViewer::keyPressEvent(QKeyEvent* event) {
		if (event->key() == Qt::Key_Control) {
			d->CtrlPressed = true;
		}
		else if (event->key() == Qt::Key_Alt) {
			d->AltPressed = true;
		}
	}

	void ImageViewer::keyReleaseEvent(QKeyEvent* event) {
		if (event->key() == Qt::Key_Control) {
			d->CtrlPressed = false;
		}
		else if (event->key() == Qt::Key_Alt) {
			d->AltPressed = false;
		}
	}

	void ImageViewer::mousePressEvent(QMouseEvent* event) {
		d->LastMousePos = event->pos();
	}

	void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
		if (event->buttons() & Qt::LeftButton) {
			QPoint delta = event->pos() - d->LastMousePos;
			d->ScrollArea->horizontalScrollBar()->setValue(d->ScrollArea->horizontalScrollBar()->value() - delta.x());
			d->ScrollArea->verticalScrollBar()->setValue(d->ScrollArea->verticalScrollBar()->value() - delta.y());
			d->LastMousePos = event->pos();
		}
	}

	void ImageViewer::mouseReleaseEvent(QMouseEvent* event) {
	
	}
}