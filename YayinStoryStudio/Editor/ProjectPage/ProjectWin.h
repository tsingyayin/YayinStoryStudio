#pragma once
#include <QtWidgets/qframe.h>

class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QVBoxLayout;

namespace YSS::ProjectPage {
	class ProjectInfoWidget;
	class ProjectWin :public QFrame {
		Q_OBJECT;
		QLabel* TitleLabel;
		QScrollArea* HistoryProjectArea;
		QWidget* HistoryProjectWidget;
		QScrollArea* ProjectTemplateArea;
		QWidget* ProjectTemplateWidget;
		QWidget* OptionWidget;
		QPushButton* OpenFolderButton;
		QPushButton* CloneGitButton;
		QVBoxLayout* ButtonLayout;
		ProjectInfoWidget* InfoWidget;
		QGridLayout* Layout;
	public:
		ProjectWin();
	private:
	};
}