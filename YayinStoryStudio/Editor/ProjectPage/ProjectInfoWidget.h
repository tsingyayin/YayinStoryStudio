#pragma once
#include <QtWidgets/qframe.h>

class QLabel;
class QGridLayout;
class QPushButton;

namespace YSSCore::General {
	class YSSProject;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget :public QFrame {
		Q_OBJECT;
	private:
		QLabel* CoverLabel;
		QLabel* TitleLabel;
		QLabel* ProjectPathLabel;
		QLabel* CreateTimeLabel;
		QLabel* LastModifiedTimeLabel;
		QLabel* SizeLabel;
		QPushButton* ShowInBrowserButton;
		QPushButton* DeleteButton;
		QGridLayout* Layout;
		YSSCore::General::YSSProject* Project = nullptr;
	public:
		ProjectInfoWidget(QWidget* parent);
		void showProject(YSSCore::General::YSSProject* project);
		void initWidget();
		void onDeleteButtonClicked();
		void onShowInBrowserButtonClicked();
	};
}