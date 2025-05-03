#pragma once
#include <QtWidgets>

namespace YSS::TitlePage {
	class TitlePage :public QFrame {
		Q_OBJECT;
		QLabel* Title;
	public:
		TitlePage();
		void showEvent(QShowEvent* event) override;
	signals:
		void startLoad();
	private:
		void load();
	};
}