#ifndef YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
#define YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
#include <General/YSSURI.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qframe.h>
#include <Widgets/ThemeManager.h>
#include <QtWidgets/qgraphicseffect.h>
#include <Widgets/BorderLabel.h>
class QLabel;
class QScrollArea;
class QWidget;
class QPushButton;
class QToolButton;
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
		struct HistoryEntry {
			YSSCore::General::YSSURI Uri;
			YSSCore::General::YSSProject* Project = nullptr;
			QString Name;
			QDateTime CreateTime;
			QDateTime LastModified;
		};
	private:
		Visindigo::Widgets::BorderLabel* TitleLabel;
		QScrollArea* HistoryProjectArea;
		QWidget* HistoryProjectWidget;
		QVBoxLayout* HistoryProjectLayout;
		QTextBrowser* NewsWidget;
		QWidget* OptionWidget;
		QToolButton* CreateProjectButton;
		QToolButton* OpenFolderButton;
		QToolButton* OpenRemoteButton;
		QPushButton* CloneGitButton;
		QToolButton* SshConnectionButton;
		QGridLayout* ButtonLayout;
		ProjectInfoWidget* InfoWidget;
		QGridLayout* Layout;
		QList<HistoryEntry> HistoryProjectList;
		QList<Visindigo::Widgets::MultiButton*> HistoryProjectLabelList;
		QMap<Visindigo::Widgets::MultiButton*, QString> HistoryProjectMap;
		QList<QLabel*> HistoryProjectTimeLabelList;
		QNetworkAccessManager* NetworkManager;
		QList<YSSCore::General::YSSProject*> HistoryLoadResults;
		QStringList HistoryLoadOrder;
		int HistoryLoadPending = 0;
		int HistoryLoadGeneration = 0;
		static bool firstOpen;
	public:
		ProjectWin();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onProjectRemoved(const YSSCore::General::YSSURI& uri);
		void onProjectSelected();
		void onProjectDoubleClicked();
		void onOpenProjectClicked();
		void onOpenRemoteProjectClicked();
		void onCreateProject();
		virtual void onThemeChanged() override;
		void onNewsMetaGot(const Visindigo::Utility::JsonConfig& jsonConfig);
	public:
		void onOpenProject(const YSSCore::General::YSSURI& uri);
	private:
		bool preCheckProject(YSSCore::General::YSSProject* prepareToOpen);
		void loadProject();
		void finishHistoryLoad(int generation);
		void refreshOptionIcons();
		int indexOfHistory(const QString& uriText) const;
		bool ensureRemoteConnected(const YSSCore::General::YSSURI& uri);
		void rememberRemoteProjectInfo(YSSCore::General::YSSProject* project);
		void pruneRemoteProjectInfo(const QStringList& keepUris);
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
