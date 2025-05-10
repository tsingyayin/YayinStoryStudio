#include "../TitlePage.h"
#include "../../GlobalValue.h"
#include <Utility/ExtTool.h>
#include "../../MainEditor/MainWin.h"
#include <chrono>
#include <thread>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qtimer.h>
namespace YSS::TitlePage {
	TitlePage::TitlePage() :QFrame() {
		this->setWindowIcon(QIcon(":/compiled/yssicon.png"));
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		this->setStyleSheet("QFrame{border-image:url(:/compiled/yayinstorystudio.png)}");
		this->setFixedSize(900, 300);

		Title = new QLabel(this);
		Title->setGeometry(0, 250, 850, 50);
		Title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		Title->setStyleSheet("QLabel{border-image:url();color:#BBBBBB}");
		Title->setText("Loading...");
	}
	void TitlePage::showEvent(QShowEvent* event) {
		QFrame::showEvent(event);
		QTimer::singleShot(1000, this, &TitlePage::load);
	}
	void TitlePage::load() {
		auto start = std::chrono::high_resolution_clock::now();
		YSS::GlobalValue* g = new YSS::GlobalValue();
		Title->setText("Preparing Environment...");
		this->repaint();
		int fontID = QFontDatabase::addApplicationFont(":/compiled/HarmonyOS_Sans_SC_Regular.ttf");
		QString hosFont = QFontDatabase::applicationFontFamilies(fontID).at(0);
		QFont font(hosFont);
		qApp->setFont(font);

		YSSCore::Utility::ExtTool::registerFileExtension("ysp", "YSS Plugin Library");
		YSS::Editor::MainWin* editor = new YSS::Editor::MainWin();
		GlobalValue::setMainWindow(editor);

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		if (duration.count() < 2000) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2000 - duration.count()));
		}
		editor->show();
		this->close();
	}
}