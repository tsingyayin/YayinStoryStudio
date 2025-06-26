#pragma once
#include <QtWidgets/qframe.h>
class QTableWidget;
class QVBoxLayout;

namespace YSS::Editor {
	class ProjectMessageViewer :public QFrame {
		Q_OBJECT;
	private:
		QTableWidget* MessageTable = nullptr;
		QVBoxLayout* Layout = nullptr;
	public:
		ProjectMessageViewer(QWidget* parent = nullptr);
		~ProjectMessageViewer();
	};
}