#pragma once
#include <QtCore/QObject>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

namespace YSS::TitlePage {
	class TitlePage :public QFrame {
		Q_OBJECT;
		QLabel* Title;
	public:
		TitlePage();
		void showEvent(QShowEvent* event) override;
	private:
		void load();
	};
}