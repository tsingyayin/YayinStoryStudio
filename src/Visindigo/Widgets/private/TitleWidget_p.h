#ifndef Visindigo_Widgets_TitleWidget_p_h
#define Visindigo_Widgets_TitleWidget_p_h
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include "Widgets/TitleWidget.h"
namespace Visindigo::__Private__ {
	class TitleWidget_MinButton :public QPushButton {
		Q_OBJECT;
	public:
		bool useSign = true;
		TitleWidget_MinButton(QWidget* parent = nullptr);
		virtual void paintEvent(QPaintEvent* event) override;
	};

	class TitleWidget_MaxButton :public QPushButton {
		Q_OBJECT;
	public:
		bool useSign = true;
		bool isMaximized = false;
		TitleWidget_MaxButton(QWidget* parent = nullptr);
		virtual void paintEvent(QPaintEvent* event) override;
	};

	class TitleWidget_CloseButton :public QPushButton {
		Q_OBJECT;
	public:
		bool useSign = true;
		TitleWidget_CloseButton(QWidget* parent = nullptr);
		virtual void paintEvent(QPaintEvent* event) override;
	};

	class TitleWidgetPrivate :public QObject{
		Q_OBJECT;
		friend class Visindigo::Widgets::TitleWidget;
		friend class TitleWidget_MinButton;
		friend class TitleWidget_MaxButton;
		friend class TitleWidget_CloseButton;
	protected:
		QColor SignColor = QColor(100, 100, 100);
		Visindigo::Widgets::TitleWidget* q;
		TitleWidget_MinButton* MinButton;
		TitleWidget_MaxButton* MaxButton;
		TitleWidget_CloseButton* CloseButton;
		QLabel* PixmapLabel;
		QLabel* TextLabel;
		QHBoxLayout* Layout = nullptr;
		QWidget* InsertWidget = nullptr;
		QWidget* TopWidget = nullptr;
		QPoint LastMousePos;
		QString PixmapPath;
		QSpacerItem* Spacer = nullptr;
		bool DragMoveEnable = true;
		Visindigo::Widgets::TitleWidget::TitleButtonFeature ButtonFeature = Visindigo::Widgets::TitleWidget::All;
		Visindigo::Widgets::TitleWidget::ButtonStyleFeature ButtonStyle = Visindigo::Widgets::TitleWidget::Win32Style;
		void setupTopWidget();
		void adjustElements();
		virtual bool eventFilter(QObject* target, QEvent* event) override;
	};

	
}
#endif // Visindigo_Widgets_TitleWidget_p_h