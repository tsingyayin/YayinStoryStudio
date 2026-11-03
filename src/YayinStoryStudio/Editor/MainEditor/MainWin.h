#ifndef YayinStoryStudio_Editor_MainEditor_MainWin_h
#define YayinStoryStudio_Editor_MainEditor_MainWin_h
#include <QObject>
#include <General/YSSURI.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmessagebox.h>
#include <Widgets/ThemeManager.h>
#include <Editor/FileServerManager.h>
class QMenuBar;
class QMenu;
class QAction;
class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QTimer;
namespace Visindigo::Widgets {
	class QuickMenu;
}
namespace YSS::Editor {
	class FileEditWidgetArea;
	class ResourceBrowser;
	class RenameDialog;
	class MainWinMenu;
	class ToolWidgetArea;
	class BottomInfoWidget;
	class TreeLayoutWidget;

	class MainWin : public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		QVBoxLayout* MainLayout = nullptr;
		QHBoxLayout* Layout = nullptr;
		RenameDialog* RenameDlg = nullptr;
		MainWinMenu* Menu = nullptr;
		TreeLayoutWidget* TreeLayout = nullptr;
		BottomInfoWidget* BottomFrame = nullptr;
		bool CloseForBack = false;
		bool CloseApproved = false;
		bool CloseAwaitingSaves = false;
		bool CloseSaveFailed = false;
		QString CloseSaveFailedUri;
		YSSCore::General::YSSURI CompareOldUri;
		YSSCore::General::YSSURI CompareNewUri;
		bool HasCompareOld = false;
		bool HasCompareNew = false;
		QElapsedTimer StartupProbeTimer;
		// 外部改动队列。项目文件永远排在最后，其余文件按到达顺序。
		QList<YSSCore::General::YSSURI> ChangedExternalList;
		// 已经向后端登记过的监视文件，关闭窗口时据此成批取消。
		QMap<QString, YSSCore::General::YSSURI> WatchedFiles;
		QTimer* ExternalChangeTimer = nullptr;
		QPointer<QMessageBox> ProjectReloadDialog;
		bool ExternalPromptRunning = false;
		bool ExternalPromptWaitingForActivation = false;
		bool ProjectReloadDialogPreempting = false;
		bool ReloadProjectAfterClose = false;
		bool ClosingDown = false;
		YSSCore::General::YSSURI ReloadProjectUri;
		static MainWin* Instance;
	private:
		YSS::Editor::FileEditWidgetArea* selectFileEditWidgetAreaForNewFile(YSSCore::Editor::FileEditWidget* widget);
		void launchComparisonIfReady();
		bool hasPendingSaves() const;
		void waitForPendingSaves();
		void watchFileForExternalChange(const YSSCore::General::YSSURI& uri);
		void unwatchFileForExternalChange(const YSSCore::General::YSSURI& uri);
		void unwatchAllFilesForExternalChange();
		void onWatchedFileChanged(const YSSCore::General::YSSURI& uri);
		void enqueueExternalChange(const YSSCore::General::YSSURI& uri);
		void processExternalChangeQueue();
		void promptReloadProject(const YSSCore::General::YSSURI& uri);
		void startProjectReload(const YSSCore::General::YSSURI& uri);
	signals:
		void currentFileEditWidgetAreaChanged(FileEditWidgetArea* area);
	public:
		MainWin();
		virtual ~MainWin();
		static MainWin* getInstance();
		void saveAllFiles(bool silent = false);
		void saveProject(bool silent = false);
		void backToHome();
		void onFileEditOpened(const YSSCore::General::YSSURI& uri);
		void saveCurrentFocusedFile();
		void saveCurrentFocusedFileAs(QString rawFilePath = "");
		void openFileDialog(const QString& startPath = "");
		void openNewFileWindow(const YSSCore::General::YSSURI& startUri = YSSCore::General::YSSURI());
		void showRenameDialog(const YSSCore::General::YSSURI& uri, bool isDirectory = false);
		void help();
		void setMenuShortcutTips();
		FileEditWidgetArea* getLastFocusedFileEditArea() const;
		void initTreeLayout();
	public:
		void setCompareFileAsOld(const YSSCore::General::YSSURI& uri);
		void setCompareFileAsNew(const YSSCore::General::YSSURI& uri);
	public slots:
		void onFileEditWidgetAreaCreated(FileEditWidgetArea* widget);
		void onFileEditWidgetAreaFocusIn(const QString& areaID);
		void onGlobalCurrentChanged(FileEditWidgetArea* area);
	public:
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void changeEvent(QEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MainWin_h
