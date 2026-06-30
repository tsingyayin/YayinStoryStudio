#include "YSS/ImageViewer.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QtMath>

#include <Utility/FileUtility.h>

namespace YSS::ImageViewer {
	class ImageViewerPrivate {
		friend class ImageViewer;
	protected:
		QGraphicsView* View = nullptr;
		QGraphicsScene* Scene = nullptr;
		QGraphicsPixmapItem* PixmapItem = nullptr;
		QPixmap Pixmap;
		QByteArray RawContent;
		bool FirstShow = true;
		double ZoomFactor = 1.15;
		double MinScale = 0.01;
		double MaxScale = 100.0;
	protected:
		void fitToWindow() {
			if (Pixmap.isNull()) return;
			View->fitInView(PixmapItem, Qt::KeepAspectRatio);

			const double scale = View->transform().m11();
			if (scale < MinScale) {
				View->resetTransform();
				View->scale(MinScale, MinScale);
			} else if (scale > MaxScale) {
				View->resetTransform();
				View->scale(MaxScale, MaxScale);
			}
		}

		void tryInitialFit(QWidget* self) {
			if (FirstShow && !Pixmap.isNull() && self->isVisible()) {
				fitToWindow();
				FirstShow = false;
			}
		}
	};

	ImageViewer::ImageViewer(QWidget* parent)
		: YSSCore::Editor::FileEditWidget(parent)
		, d(new ImageViewerPrivate())
	{
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		d->Scene = new QGraphicsScene(this);
		d->PixmapItem = new QGraphicsPixmapItem();
		d->Scene->addItem(d->PixmapItem);

		d->View = new QGraphicsView(d->Scene, this);
		d->View->setRenderHint(QPainter::SmoothPixmapTransform);
		d->View->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		d->View->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
		d->View->setDragMode(QGraphicsView::ScrollHandDrag);
		d->View->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
		d->View->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->View->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->View->setFrameShape(QFrame::NoFrame);
		d->View->setBackgroundBrush(QColor(45, 45, 45));

		layout->addWidget(d->View);
		setLayout(layout);
	}

	ImageViewer::~ImageViewer() {
		delete d;
	}

	bool ImageViewer::onOpen(const QString& filePath) {
		d->RawContent = Visindigo::Utility::FileUtility::readByteArray(filePath);
		d->Pixmap = QPixmap();
		if (!d->Pixmap.loadFromData(d->RawContent)) {
			return false;
		}

		d->PixmapItem->setPixmap(d->Pixmap);
		d->Scene->setSceneRect(d->Pixmap.rect());

		emit imageSizeObtained(d->Pixmap.size());

		d->FirstShow = true;
		d->tryInitialFit(this);

		return true;
	}

	bool ImageViewer::onClose() {
		d->PixmapItem->setPixmap(QPixmap());
		d->Pixmap = QPixmap();
		d->RawContent.clear();
		return true;
	}

	bool ImageViewer::onSave(const QString& filePath) {
		Visindigo::Utility::FileUtility::saveByteArray(filePath, d->RawContent);
		return true;
	}

	void ImageViewer::showEvent(QShowEvent* event) {
		YSSCore::Editor::FileEditWidget::showEvent(event);
		setFocus();
		d->tryInitialFit(this);
	}

	void ImageViewer::resizeEvent(QResizeEvent* event) {
		YSSCore::Editor::FileEditWidget::resizeEvent(event);
		d->tryInitialFit(this);
	}

	void ImageViewer::wheelEvent(QWheelEvent* event) {
		double angle = event->angleDelta().y();
		if (qFuzzyIsNull(angle)) return;

		double factor = (angle > 0) ? d->ZoomFactor : (1.0 / d->ZoomFactor);
		double currentScale = d->View->transform().m11();
		double newScale = currentScale * factor;

		if (newScale < d->MinScale || newScale > d->MaxScale) return;

		d->View->scale(factor, factor);
		event->accept();
	}

	void ImageViewer::mouseDoubleClickEvent(QMouseEvent* event) {
		d->fitToWindow();
		event->accept();
	}

}