#include "Widgets/UpToWin11.h"
#include <Utility/FileUtility.h>
namespace YSS::Widgets {
	UpToWin11::UpToWin11() {
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
		this->setFixedSize(640, 360); // 16:9 ratio
		this->setAttribute(Qt::WA_TranslucentBackground);
		this->setWindowIcon(QIcon());
		this->setWindowTitle(" ");
		this->setStyleSheet(R"(
			QFrame{
				color: #000000;
			}
		)");
		BackgroundLabel = new QLabel(this);
		BackgroundLabel->resize(this->size());
		BackgroundLabel->setStyleSheet(R"(
			QLabel{
				border-image: url(:/resource/cn.yxgeneral.yayinstorystudio/icon/up_to_win11.jpg);
				border-radius: 6px;
			}
		)");
		BackgroundImage = QImage(":/resource/cn.yxgeneral.yayinstorystudio/icon/up_to_win11.jpg");
		BackgroundImage = BackgroundImage.scaled(640, 360, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

		TitleBackgroundLabel = new QLabel(this);
		TitleBackgroundLabel->setGeometry(0, 0, this->width(), 40);
		TitleLabel = new QLabel(this);
		TitleLabel->setGeometry(TitleBackgroundLabel->geometry());
		TitleLabel->setTextFormat(Qt::RichText);
		TitleLabel->setText(R"(<font color="black">你值得拥有 Windows 11</font>)");
		TitleLabel->setContentsMargins(20, 5, 20, 5);
		TitleLabel->setAlignment(Qt::AlignCenter);

		TitleEffect = new Visindigo::Widgets::LiquidGlassEffect(TitleBackgroundLabel);
		TitleEffect->setBackgroundPolicy(Visindigo::Widgets::LiquidGlassEffect::BackgroundPolicy::CustomImage);
		TitleEffect->setPositionPolicy(Visindigo::Widgets::LiquidGlassEffect::PositionPolicy::ParentLocalGeometry);
		TitleEffect->setColorMask(QColor("#FFFFFF"), 0.7);
		TitleEffect->setBlurRadius(80);
		TitleEffect->setBorderRadius(6);
		TitleEffect->setLiquidDistortRadius(20);
		TitleEffect->setBackgroundImage(BackgroundImage);
		TitleBackgroundLabel->setGraphicsEffect(TitleEffect);

		BottomFrame = new QFrame(this);
		BottomFrame->setGeometry(0, this->height() - 60, this->width(), 60);
		BottomBackgroundLabel = new QLabel(BottomFrame);
		BottomBackgroundLabel->resize(BottomFrame->size());
		BottomEffect = new Visindigo::Widgets::LiquidGlassEffect(BottomBackgroundLabel);
		BottomEffect->setBackgroundPolicy(Visindigo::Widgets::LiquidGlassEffect::BackgroundPolicy::CustomImage);
		BottomEffect->setPositionPolicy(Visindigo::Widgets::LiquidGlassEffect::PositionPolicy::ParentLocalGeometry);
		BottomEffect->setColorMask(QColor("#FFFFFF"), 0.7);
		BottomEffect->setBlurRadius(80);
		BottomEffect->setBorderRadius(6);
		BottomEffect->setLiquidDistortRadius(20);
		BottomEffect->setBackgroundImage(BackgroundImage);
		BottomBackgroundLabel->setGraphicsEffect(BottomEffect);

		TextLabel = new QLabel(BottomFrame);
		ViewButton = new QPushButton(BottomFrame);
		CancelButton = new QPushButton(BottomFrame);
		TextLabel->setTextFormat(Qt::RichText);
		TextLabel->setText(R"(Yayin Story Studio 使用现代操作系统API构建，使用Windows11可以获得更好的外观效果。)");
		TextLabel->setWordWrap(true);
		TextLabel->setMinimumWidth(this->width() * 0.6);
		ViewButton->setText("查看详情");
		ViewButton->setMinimumHeight(30);
		ViewButton->setStyleSheet(R"(
			QPushButton{
				background-color: #0078D7;
				color: white;
				border: none;
				border-radius: 4px;
				padding: 5px 15px;
			}
			QPushButton:hover{
				background-color: #005A9E;
			}
			QPushButton:pressed{
				background-color: #003E6B;
			}
		)");
		CancelButton->setText("算了");
		CancelButton->setStyleSheet(R"(
			QPushButton{
				background-color: #FFFFFF;
				color: black;
				border: none;
				border-radius: 4px;
				padding: 5px 15px;
			}
			QPushButton:hover{
				background-color: #C8C8C8;
			}
			QPushButton:pressed{
				background-color: #AFAFAF;
			}
		)");
		CancelButton->setMinimumHeight(30);
		BottomLayout = new QHBoxLayout(BottomFrame);
		BottomLayout->addWidget(TextLabel);
		BottomLayout->addWidget(CancelButton);
		BottomLayout->addWidget(ViewButton);

		connect(CancelButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});
		connect(ViewButton, &QPushButton::clicked, this, [this]() {
			Visindigo::Utility::FileUtility::openBrowser("https://www.microsoft.com/windows/windows-11");
			this->close();
			});
	}
}