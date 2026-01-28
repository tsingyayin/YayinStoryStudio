#pragma once
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <Widgets/ThemeManager.h>
#include <QtWidgets/qgraphicseffect.h>

class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
class QRadioButton;

namespace YSSCore::General {
	class YSSProject;
}
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Widgets {
	class MultiButton;
	class TitleWidget;
	class WidgetResizeTool;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget;
	class ProjectWin :public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		Visindigo::Utility::JsonConfig* Config;
		Visindigo::Widgets::TitleWidget* TitleWidget;
		Visindigo::Widgets::WidgetResizeTool* ResizeTool;
		QFrame* BGFrame;
		QGraphicsOpacityEffect* OpacityEffect;
		QLabel* TitleLabel;
		QScrollArea* HistoryProjectArea;
		QWidget* HistoryProjectWidget;
		QVBoxLayout* HistoryProjectLayout;
		QWidget* NewsWidget;
		QWidget* OptionWidget;
		QPushButton* CreateProjectButton;
		QPushButton* OpenFolderButton;
		QPushButton* CloneGitButton;
		QRadioButton* ThemeLightButton;
		QVBoxLayout* ButtonLayout;
		ProjectInfoWidget* InfoWidget;
		QGridLayout* Layout;
		QList<YSSCore::General::YSSProject*> HistoryProjectList;
		QList<Visindigo::Widgets::MultiButton*> HistoryProjectLabelList;
		QMap<Visindigo::Widgets::MultiButton*, YSSCore::General::YSSProject*> HistoryProjectMap;
		QList<QLabel*> HistoryProjectTimeLabelList;
	public:
		ProjectWin();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onProjectRemoved(YSSCore::General::YSSProject* project);
		void onProjectSelected();
		void onProjectDoubleClicked();
		void onOpenProjectClicked();
		void onOpenProject(QString projectPath = "");
		void onCreateProject();
		virtual void onThemeChanged() override;
	private:
		void loadProject();
	};
}