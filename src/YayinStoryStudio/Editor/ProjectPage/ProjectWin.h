#ifndef YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
#define YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <Widgets/ThemeManager.h>
#include <QtWidgets/qgraphicseffect.h>
#include <Widgets/BorderLabel.h>
class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
class QRadioButton;
class QTextBrowser;
class QNetworkAccessManager;
namespace YSSCore::General {
	class YSSProject;
}
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Widgets {
	class MultiButton;
	class MultiButtonGroup;
	class TitleWidget;
	class WidgetResizeTool;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget;
	class ProjectWin :public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		Visindigo::Widgets::BorderLabel* TitleLabel;
		QScrollArea* HistoryProjectArea;
		QWidget* HistoryProjectWidget;
		QVBoxLayout* HistoryProjectLayout;
		QTextBrowser* NewsWidget;
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
		QNetworkAccessManager* NetworkManager;
		static bool firstOpen;
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
		void onNewsMetaGot(const Visindigo::Utility::JsonConfig& jsonConfig);
	private:
		// 只做检查与询问，不接管 prepareToOpen 的所有权；返回true表示可以继续打开。
		bool preCheckProject(YSSCore::General::YSSProject* prepareToOpen);
		void loadProject();
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
