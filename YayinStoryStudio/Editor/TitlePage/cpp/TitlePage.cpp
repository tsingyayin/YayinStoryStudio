#include "../TitlePage.h"
#include "../../GlobalValue.h"
#include <Utility/ExtTool.h>
#include "../../MainEditor/MainWin.h"
#include <chrono>
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
		connect(this, &TitlePage::startLoad, this, &TitlePage::load, Qt::QueuedConnection);
	}
	void TitlePage::showEvent(QShowEvent* event) {
		QFrame::showEvent(event);
		emit startLoad();
	}
	void TitlePage::load() {

		auto start = std::chrono::high_resolution_clock::now();
		Title->setText("Loading...");
		this->repaint();
		YSS::GlobalValue* g = new YSS::GlobalValue();
		Title->setText("Prepare Environment...");
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
		if (duration.count() < 1000) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 - duration.count()));
		}
		editor->show();
		this->close();
	}
}