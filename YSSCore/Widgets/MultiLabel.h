#pragma once
#include <QtWidgets/qframe.h>
#include "../Macro.h"
class QLabel;
class QString;
class QIcon;
class QPixmap;
namespace YSSCore::Widgets {
	class MultiLabelPrivate;
	class YSSCoreAPI MultiLabel :public QFrame {
	public:
		MultiLabel(QWidget* parent = nullptr);
		virtual ~MultiLabel();
		void setTitle(const QString& str);
		void setDescription(const QString& str);
		void setPixmap(const QPixmap& pixmap);
		QString getTitle();
		QString getDescription();
		QPixmap getPixmap();
		void setContentsMargins(int left, int top, int right, int bottom);
		void setSpacing(int spaceing);
		void setAlignment(Qt::Alignment alignment);
		void setPixmapFixedWidth(int width);
	private:
		MultiLabelPrivate* d;
	};
}