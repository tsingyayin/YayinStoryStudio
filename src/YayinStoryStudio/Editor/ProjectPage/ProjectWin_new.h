#ifndef YayinStoryStudio_Editor_ProjectPage_ProjectWin_new_h
#define YayinStoryStudio_Editor_ProjectPage_ProjectWin_new_h
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtGui/qpixmap.h>
#include <Widgets/ThemeManager.h>
class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QTextBrowser;
class QNetworkAccessManager;
namespace YSSCore::General {
	class YSSProject;
}
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget_new;
	class ProjectCard;
	class ProjectWin_new :public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QLabel* TitleLabel = nullptr;
		QLabel* SubtitleLabel = nullptr;
		QLabel* StatusLabel = nullptr;
		QLabel* RecordHeaderLabel = nullptr;
		QLabel* EmptyRailLabel = nullptr;
		QScrollArea* HistoryProjectArea = nullptr;
		QWidget* HistoryProjectWidget = nullptr;
		QVBoxLayout* HistoryProjectLayout = nullptr;
		QTextBrowser* NewsWidget = nullptr;
		QWidget* OptionWidget = nullptr;
		QPushButton* CreateProjectButton = nullptr;
		QPushButton* OpenFolderButton = nullptr;
		QHBoxLayout* ButtonLayout = nullptr;
		ProjectInfoWidget_new* InfoWidget = nullptr;
		QGridLayout* Layout = nullptr;
		QList<YSSCore::General::YSSProject*> HistoryProjectList;
		QList<ProjectCard*> HistoryProjectCardList;
		QMap<ProjectCard*, YSSCore::General::YSSProject*> HistoryProjectMap;
		QNetworkAccessManager* NetworkManager = nullptr;
		QList<YSSCore::General::YSSProject*> HistoryLoadResults;
		QLabel* BackgroundLabel = nullptr;
		QWidget* BackdropOverlay = nullptr;
		QPixmap BackgroundSource;
		YSSCore::General::YSSProject* SelectedProject = nullptr;
		bool LayoutArcGuard = false;
		int HistoryLoadPending = 0;
		int HistoryLoadGeneration = 0;
		static bool firstOpen;
	public:
		ProjectWin_new();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onProjectRemoved(YSSCore::General::YSSProject* project);
		void onOpenProjectClicked();
		void onOpenProject(QString projectPath = "");
		void onCreateProject();
		virtual void onThemeChanged() override;
		void onNewsMetaGot(const Visindigo::Utility::JsonConfig& jsonConfig);
	private:
		// 只做检查与询问，不接管 prepareToOpen 的所有权；返回true表示可以继续打开。
		bool preCheckProject(YSSCore::General::YSSProject* prepareToOpen);
		void loadProject();
		void finishHistoryLoad(int generation);
		void applyArchiveTheme();
		void updateProjectCardLayout();
		void updateRailSummary();
		void updateBackdropPixmap();
		void selectProjectCard(ProjectCard* card);
		void openProjectCard(ProjectCard* card);
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectWin_new_h
