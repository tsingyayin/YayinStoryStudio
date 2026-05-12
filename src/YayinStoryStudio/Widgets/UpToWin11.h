#pragma once
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <Widgets/LiquidGlassEffect.h>
#include <QtGui/qimage.h>
namespace YSS::Widgets {
	class UpToWin11 :public QFrame {
		Q_OBJECT;
	public:
		UpToWin11();
		QLabel* BackgroundLabel;
		QLabel* TitleBackgroundLabel;
		QLabel* TitleLabel;
		QLabel* BottomBackgroundLabel;
		QFrame* BottomFrame;
		QLabel* TextLabel;
		QPushButton* ViewButton;
		QPushButton* CancelButton;
		QHBoxLayout* BottomLayout;
		QImage BackgroundImage;
		Visindigo::Widgets::LiquidGlassEffect* TitleEffect;
		Visindigo::Widgets::LiquidGlassEffect* BottomEffect;
	};
}