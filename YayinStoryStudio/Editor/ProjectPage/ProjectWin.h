#pragma once
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>

class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QVBoxLayout;

namespace YSSCore::General {
	class YSSProject;
}
namespace YSSCore::Utility {
	class JsonConfig;
}
namespace YSSCore::Widgets {
	class MultiButton;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget;
	class ProjectWin :public QFrame {
		Q_OBJECT;
	private:
		YSSCore::Utility::JsonConfig* Config;
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
		QList<YSSCore::General::YSSProject*> HistoryProjectList;
		QList<YSSCore::Widgets::MultiButton*> HistoryProjectLabelList;
		QMap<YSSCore::Widgets::MultiButton*, YSSCore::General::YSSProject*> HistoryProjectMap;
		QList<QLabel*> HistoryProjectTimeLabelList;
	public:
		ProjectWin();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onProjectSelected();
	private:
		void loadProject();
	};
}