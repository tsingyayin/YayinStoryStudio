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
	signals:
		void removeConfirmed(YSSCore::General::YSSProject* who);
	private:
		QLabel* CoverLabel;
		QLabel* TitleLabel;
		QLabel* SizeLabel;
		QLabel* CreateTimeLabel;
		QLabel* LastModifiedTimeLabel;
		QPushButton* ShowInBrowserButton;
		QPushButton* RemoveFromListButton;
		QPushButton* DeleteButton;
		QGridLayout* Layout;
		YSSCore::General::YSSProject* Project = nullptr;
	public:
		ProjectInfoWidget(QWidget* parent);
		void showProject(YSSCore::General::YSSProject* project);
		void initWidget();
		void onDeleteButtonClicked();
		void onShowInBrowserButtonClicked();
		void onRemoveFromListButtonClicked();
		virtual void resizeEvent(QResizeEvent* event) override;
	};
}