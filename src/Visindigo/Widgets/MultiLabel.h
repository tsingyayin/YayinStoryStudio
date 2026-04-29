#pragma once
#include <QtWidgets/qframe.h>
#include "../VICompileMacro.h"
#include "Widgets/BorderFrame.h"
//// Forward declarations
class QLabel;
class QString;
class QIcon;
class QPixmap;
namespace Visindigo::Widgets {
	class MultiLabelPrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI MultiLabel :public BorderFrame {
	public:
		MultiLabel(QWidget* parent = nullptr);
		virtual ~MultiLabel();
		void setTitle(const QString& str);
		void setDescription(const QString& str);
		void setPixmapPath(const QString& filePath);
		QString getTitle();
		QString getDescription();
		QString getPixmapPath();
		void setContentsMargins(int left, int top, int right, int bottom);
		void setSpacing(int spaceing);
		void setAlignment(Qt::Alignment alignment);
		void setPixmapFixedWidth(int width);
		void addCustomWidget(QWidget* widget);
		void setCustomWidgetOrientation(Qt::Orientation orientation);
		virtual void resizeEvent(QResizeEvent* event) override;
		QWidget* getCustomWidget();
	private:
		MultiLabelPrivate* d;
	};
}