#pragma once
#include <QFrame>

class QLabel;
class QGridLayout;

namespace YSS::ProjectPage {
	class HistoryProjectLabel :public QFrame {
		Q_OBJECT;
	private:
		QLabel* ProjectIcon = nullptr;
		QLabel* ProjectName = nullptr;
		QLabel* ProjectPath = nullptr;
		QLabel* ProjectTime = nullptr;
		QGridLayout* Layout = nullptr;
	public:
		HistoryProjectLabel(QWidget* parent = nullptr);
		~HistoryProjectLabel();
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	};
}