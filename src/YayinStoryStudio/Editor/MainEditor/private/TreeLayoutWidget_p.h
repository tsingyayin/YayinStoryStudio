#ifndef YSS_MainEditor_TreeLayoutWidget_p_h
#define YSS_MainEditor_TreeLayoutWidget_p_h
#include <QtWidgets/qwidget.h>
#include <QtGui/qpainter.h>
#include <QtGui/qcolor.h>
#include <QtGui/qregion.h>
#include <QtGui/qpainterpath.h>
namespace YSS::Editor {
	class TreeLayoutWidgetPrivate;
	class TreeLayoutDropZone :public QWidget {
		Q_OBJECT;
	public:
		enum class DropZone : qint8 {
			None = 0,
			OuterLeft,
			OuterRight,
			OuterTop,
			OuterBottom,
			ItemTop,
			ItemBottom,
			ItemLeft,
			ItemRight,
			Seam,
		};
	public:
		TreeLayoutDropZone(TreeLayoutWidgetPrivate* owner, DropZone zone, int childIndex);
		void setZoneShape(const QPainterPath& shape);
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dragMoveEvent(QDragMoveEvent* event) override;
		virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;
	private:
		void setHovered(bool hovered);
	private:
		TreeLayoutWidgetPrivate* Owner;
		DropZone Zone;
		int ChildIndex;
		QPainterPath Shape;
		bool Hovered = false;
	};
}
#endif // YSS_MainEditor_TreeLayoutWidget_p_h