#pragma once
#include <QFrame>

namespace YSS::ProjectPage {
	class HistoryProjectLabel;
	class HistoryProjectWidgetPrivate;
	class HistoryProjectWidget :public QFrame {
		Q_OBJECT;
	public:
		QList<HistoryProjectLabel*> HistoryProjectList;
	public:
		HistoryProjectWidget(QWidget* parent = nullptr);
		~HistoryProjectWidget();
	};
}