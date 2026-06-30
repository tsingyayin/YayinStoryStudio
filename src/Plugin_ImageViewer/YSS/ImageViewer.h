#ifndef ImageViewer_YSS_ImageViewer_H
#define ImageViewer_YSS_ImageViewer_H
#include <Editor/FileEditWidget.h>

namespace YSS::ImageViewer {

	class ImageViewerPrivate;

	/// 基于 QGraphicsView 的图片查看器，支持：
	/// - 滚轮缩放（以鼠标位置为中心）
	/// - 左键拖拽平移
	/// - 默认自动适应窗口大小
	/// - 双击恢复适合窗口
	class ImageViewer : public YSSCore::Editor::FileEditWidget {
		Q_OBJECT
	signals:
		void imageSizeObtained(const QSize& size);
	public:
		explicit ImageViewer(QWidget* parent = nullptr);
		virtual ~ImageViewer() override;
	public:
		virtual bool onOpen(const QString& filePath) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& filePath = "") override;
	protected:
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	private:
		ImageViewerPrivate* d;
	};

} // namespace YSS::ImageViewer
#endif // ImageViewer_YSS_ImageViewer_H
