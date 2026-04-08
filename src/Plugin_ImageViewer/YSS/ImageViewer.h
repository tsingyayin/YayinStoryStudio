#ifndef ImageViewer_YSS_ImageViewer_H
#define ImageViewer_YSS_ImageViewer_H
#include <Editor/FileEditWidget.h>

namespace YSS::ImageViewer {
	class ImageViewerPrivate;
	class ImageViewer :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	public:
		ImageViewer(QWidget* parent = nullptr);
		virtual ~ImageViewer();
		virtual bool onOpen(const QString& filePath) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& filePath = "") override;
	public:
		virtual void showEvent(QShowEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void keyReleaseEvent(QKeyEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
	signals:
		void imageSizeObtained(const QSize& size);
	private:
		ImageViewerPrivate* d;
	};
}
#endif // ImageViewer_YSS_ImageViewer_H
