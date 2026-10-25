#include <QtGui/qevent.h>
#include <QtGui/qtransform.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtMath>
#include <General/Log.h>
#include <Utility/FileOperation.h>
#include "YSS/ImageViewer.h"

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
		// 缩小显示时由 QPainter 直接缩放只有双线性点采样，会出现摩尔纹和细节丢失，
		// 因此缩小到一定程度后改为预滤波重采样（与专用看图软件一致）。
		static constexpr double PrescaleThreshold = 0.9;
		static constexpr double PrescaleTolerance = 0.1;
		enum class DisplayMode { Original, Filtered };
		DisplayMode Mode = DisplayMode::Original;
		QSize DisplayedSize;
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
			updateDisplay();
		}

		void tryInitialFit(QWidget* self) {
			if (FirstShow && !Pixmap.isNull() && self->isVisible()) {
				fitToWindow();
				FirstShow = false;
			}
		}

		// 按当前缩放倍率与设备像素比重建显示缓冲：
		// 放大时不重采样（保留原始像素，保持锐利），缩小时用高质量滤波器预缩放，
		// 再以 1:1 设备像素绘制，避免 QGraphicsView 绘制阶段的走样。
		void updateDisplay() {
			if (Pixmap.isNull()) return;

			const double scale = View->transform().m11();
			if (scale <= 0) return;

			const double dpr = View->devicePixelRatioF();
			if (scale >= 1.0) {
				showOriginal(Qt::FastTransformation);
				return;
			}
			if (scale > PrescaleThreshold) {
				showOriginal(Qt::SmoothTransformation);
				return;
			}

			const QSize target(qMax(1, qRound(Pixmap.width() * scale * dpr)),
				qMax(1, qRound(Pixmap.height() * scale * dpr)));
			// 高 DPI 下 0.9~1.0 倍的目标尺寸可能超过原图，放大交给绘制阶段即可，避免多余的大图分配。
			if (target.width() >= Pixmap.width() || target.height() >= Pixmap.height()) {
				showOriginal(Qt::SmoothTransformation);
				return;
			}
			// 允许缓存缓冲与目标尺寸存在一定偏差，避免滚轮缩放的每一格都重新分配大图。
			if (Mode == DisplayMode::Filtered && DisplayedSize.isValid()
				&& target.width() * (1.0 + PrescaleTolerance) >= DisplayedSize.width()
				&& target.width() * (1.0 - PrescaleTolerance) <= DisplayedSize.width()) {
				return;
			}

			QPixmap scaled = Pixmap.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			if (scaled.isNull()) return;
			scaled.setDevicePixelRatio(dpr);

			Mode = DisplayMode::Filtered;
			DisplayedSize = scaled.size();
			PixmapItem->setTransformationMode(Qt::SmoothTransformation);
			PixmapItem->setPixmap(scaled);
			// 让缩放后的缓冲仍然精确覆盖原始场景尺寸（含整数取整带来的亚像素误差），
			// 保证 fitInView / 拖拽 / 缩放中心计算不受影响。
			PixmapItem->setTransform(QTransform::fromScale(
				qreal(Pixmap.width()) * dpr / scaled.width(),
				qreal(Pixmap.height()) * dpr / scaled.height()));
		}

		void showOriginal(Qt::TransformationMode mode) {
			if (Mode == DisplayMode::Original && PixmapItem->transformationMode() == mode) return;
			Mode = DisplayMode::Original;
			DisplayedSize = QSize();
			PixmapItem->setTransformationMode(mode);
			PixmapItem->setPixmap(Pixmap);
			PixmapItem->setTransform(QTransform());
		}

		void resetDisplay() {
			Mode = DisplayMode::Original;
			DisplayedSize = QSize();
			PixmapItem->setTransform(QTransform());
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
		Visindigo::Utility::FileOperation::Errorable<QByteArray> contentResult = Visindigo::Utility::FileOperation::readBinary(filePath);
		if (not contentResult) {
			vgErrorF << "Failed to read image file: " << filePath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(contentResult.error());
			return false;
		}
		d->RawContent = contentResult.value();
		d->Pixmap = QPixmap();
		if (!d->Pixmap.loadFromData(d->RawContent)) {
			return false;
		}

		d->PixmapItem->setPixmap(d->Pixmap);
		d->Scene->setSceneRect(d->Pixmap.rect());
		d->resetDisplay();

		emit imageSizeObtained(d->Pixmap.size());

		d->FirstShow = true;
		d->tryInitialFit(this);

		return true;
	}

	bool ImageViewer::onClose() {
		d->PixmapItem->setPixmap(QPixmap());
		d->Pixmap = QPixmap();
		d->RawContent.clear();
		d->resetDisplay();
		return true;
	}

	bool ImageViewer::onSave(const QString& filePath) {
		Visindigo::Utility::FileOperation::ErrorCode saveResult = Visindigo::Utility::FileOperation::saveBinary(filePath, d->RawContent);
		if (saveResult != Visindigo::Utility::FileOperation::Success) {
			vgErrorF << "Failed to save image file: " << filePath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(saveResult);
			return false;
		}
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
		d->updateDisplay();
		event->accept();
	}

	void ImageViewer::mouseDoubleClickEvent(QMouseEvent* event) {
		d->fitToWindow();
		event->accept();
	}

}