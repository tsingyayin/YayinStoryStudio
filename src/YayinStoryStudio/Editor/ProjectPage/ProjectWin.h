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
		// 历史项目的一条记录。本地项目总是带着已加载的 Project；远程项目在未连接时
		// 不会真正加载，此时 Project 为空，名称与时间来自 RecentRemoteProjectInfo 缓存。
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
		QPushButton* CreateProjectButton;
		QPushButton* OpenFolderButton;
		QPushButton* CloneGitButton;
		// 临时入口：等 ResourceBrowser 接通 UFS 后，改由需要远端凭据的地方调用
		// SSHConnectionWidget::requestVerification()。
		QPushButton* SshConnectionButton;
		QRadioButton* ThemeLightButton;
		QVBoxLayout* ButtonLayout;
		ProjectInfoWidget* InfoWidget;
		QGridLayout* Layout;
		QList<HistoryEntry> HistoryProjectList;
		QList<Visindigo::Widgets::MultiButton*> HistoryProjectLabelList;
		// 标签 -> 项目 URI。远端条目可能没有 Project 对象，所以这里记 URI 而不是对象。
		QMap<Visindigo::Widgets::MultiButton*, QString> HistoryProjectMap;
		QList<QLabel*> HistoryProjectTimeLabelList;
		QNetworkAccessManager* NetworkManager;
		QList<YSSCore::General::YSSProject*> HistoryLoadResults;
		// 本次加载批次要处理的 URI 顺序，来自 RecentProjects。
		QStringList HistoryLoadOrder;
		int HistoryLoadPending = 0;
		int HistoryLoadGeneration = 0;
		static bool firstOpen;
	public:
		ProjectWin();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		// 把该 URI 从历史列表里移除。本地项目由信息面板取 YSSProject 的 URI 后走到这里。
		void onProjectRemoved(const YSSCore::General::YSSURI& uri);
		void onProjectSelected();
		void onProjectDoubleClicked();
		void onOpenProjectClicked();
		void onCreateProject();
		virtual void onThemeChanged() override;
		void onNewsMetaGot(const Visindigo::Utility::JsonConfig& jsonConfig);
	public:
		// 打开项目。uri 可以是本地路径或远端地址，由 YSSURI 统一表达。
		void onOpenProject(const YSSCore::General::YSSURI& uri);
	private:
		// 只做检查与询问，不接管 prepareToOpen 的所有权；返回true表示可以继续打开。
		bool preCheckProject(YSSCore::General::YSSProject* prepareToOpen);
		void loadProject();
		void finishHistoryLoad(int generation);
		// 历史列表里给定 URI 的条目下标，找不到返回 -1。
		int indexOfHistory(const QString& uriText) const;
		// 远端项目未连接时先要凭据；返回 false 表示现在还不能加载。
		bool ensureRemoteConnected(const YSSCore::General::YSSURI& uri);
		// 把已加载的远端项目信息写进 RecentRemoteProjectInfo 缓存。
		void rememberRemoteProjectInfo(YSSCore::General::YSSProject* project);
		// 丢弃 RecentRemoteProjectInfo 里不再被 RecentProjects 引用的记录。
		void pruneRemoteProjectInfo(const QStringList& keepUris);
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectWin_h
