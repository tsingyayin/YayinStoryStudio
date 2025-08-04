#pragma once
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QVBoxLayout;

namespace Visindigo::General {
	class YSSProject;
}
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Widgets {
	class MultiButton;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget;
	class ProjectWin :public QFrame {
		Q_OBJECT;
	private:
		Visindigo::Utility::JsonConfig* Config;
		QLabel* TitleLabel;
		QScrollArea* HistoryProjectArea;
		QWidget* HistoryProjectWidget;
		QVBoxLayout* HistoryProjectLayout;
		QWidget* NewsWidget;
		QWidget* OptionWidget;
		QPushButton* CreateProjectButton;
		QPushButton* OpenFolderButton;
		QPushButton* CloneGitButton;
		QVBoxLayout* ButtonLayout;
		ProjectInfoWidget* InfoWidget;
		QGridLayout* Layout;
		QList<Visindigo::General::YSSProject*> HistoryProjectList;
		QList<Visindigo::Widgets::MultiButton*> HistoryProjectLabelList;
		QMap<Visindigo::Widgets::MultiButton*, Visindigo::General::YSSProject*> HistoryProjectMap;
		QList<QLabel*> HistoryProjectTimeLabelList;
	public:
		ProjectWin();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onProjectRemoved(Visindigo::General::YSSProject* project);
		void onProjectSelected();
		void onProjectDoubleClicked();
		void onOpenProjectClicked();
		void onOpenProject(QString projectPath = "");
		void onCreateProject();
	private:
		void loadProject();
	};
}