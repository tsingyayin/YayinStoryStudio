#include <chrono>
#include <thread>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qtimer.h>
#include <Utility/FileUtility.h>
#include <Utility/ExtTool.h>
#include "../TitlePage.h"
#include "../../GlobalValue.h"
#include "../../MainEditor/MainWin.h"
#include "../../ProjectPage/ProjectWin.h"
#include "../../../test_main.h"
namespace YSS::TitlePage {
	TitlePage::TitlePage() :QFrame() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		this->setStyleSheet("QFrame{border-image:url(:/yss/compiled/yayinstorystudio.png)}");
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
		Title->setText("Preparing Environment...");
		this->repaint();
		YSS::GlobalValue* g = new YSS::GlobalValue();
		int fontID = QFontDatabase::addApplicationFont(":/yss/compiled/HarmonyOS_Sans_SC_Regular.ttf");
		QString hosFont = QFontDatabase::applicationFontFamilies(fontID).at(0);
		QFont font(hosFont);
		qApp->setFont(font);

		Visindigo::Utility::ExtTool::registerFileExtension("yst", "YSS StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtension("ysp", "YSS Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtension("yssp", "YSS Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
		YSS::Editor::MainWin* mainWin = new YSS::Editor::MainWin();
		GlobalValue::setMainWindow(mainWin);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		if (duration.count() < 1000) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 - duration.count()));
		}
#ifdef DEBUG
		test_loaded();
#endif
		this->close();
	}
}