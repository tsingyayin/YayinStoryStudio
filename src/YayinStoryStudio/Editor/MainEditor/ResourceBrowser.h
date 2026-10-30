#ifndef YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
#define YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlist.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qcolor.h>
#include <QtWidgets/qwidget.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
#include <General/YSSURI.h>
#include <General/YSSUniqueFileSystem.h>
class QAction;
class QMenu;
class QToolBar;
class QTreeView;
class QVBoxLayout;
namespace YSS::Editor {
	class UfsTreeModel;
	class ResourceBrowserVFS :public YSSCore::Editor::FileServer {
		Q_OBJECT;
	public:
		ResourceBrowserVFS(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class ResourceBrowser :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	private:
		QTreeView* FileTree = nullptr;
		UfsTreeModel* FileModel = nullptr;
		QVBoxLayout* Layout = nullptr;
		QToolBar* ToolBar = nullptr;
		QAction* ToolActionRefresh = nullptr;
		QAction* ToolActionExpand = nullptr;
		QAction* ToolActionShrink = nullptr;
		QAction* ToolActionNewFile = nullptr;
		QAction* ToolActionNewFolder = nullptr;
		QColor TextColor;
		QMenu* FileOptions = nullptr;
		YSSCore::General::YSSURI ProjectRootUri;
		YSSCore::General::YSSURI CurrentUri;
		QPersistentModelIndex RightClickedIndex;
		QAction* FileOptionOpen = nullptr;
		QAction* FileOptionRename = nullptr;
		QAction* FileOptionDelete = nullptr;
		QAction* FileOptionNewFile = nullptr;
		QAction* FileOptionNewFolder = nullptr;
		QAction* FileOptionCopyPath = nullptr;
		QAction* FileOptionCopyName = nullptr;
		QAction* FileOptionCopy = nullptr;
		QAction* FileOptionPaste = nullptr;
		QAction* FileOptionCut = nullptr;
		QAction* FileOptionRefresh = nullptr;
		QAction* FileOptionCompareAsOld = nullptr;
		QAction* FileOptionCompareAsNew = nullptr;
		QList<YSSCore::General::YSSURI> ClipboardUris;
		bool ClipboardCut = false;
	signals:
		void visibilityChanged(bool visible);
		// 请求主窗口弹出重命名对话框；isDirectory 决定对话框是否做扩展名处理。
		void renameRequested(const YSSCore::General::YSSURI& uri, bool isDirectory);
	public:
		static ResourceBrowser* getInstance();
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		virtual ~ResourceBrowser();
		void refresh();
		void setCurrentSelected(const YSSCore::General::YSSURI& uri);
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) override;
	private:
		void onThemeChanged();
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
		void onFileTreeContextMenuRequested(const QPoint& pos);
		void onRenameTriggered();
		void onNewFolderTriggered();
		void onDeleteTriggered();
		void onCopyTriggered(bool cut);
		void onPasteTriggered();
		YSSCore::General::YSSURI targetDirectory() const;
		void selectAndExpand(const QModelIndex& index);
		void refreshParentOf(const YSSCore::General::YSSURI& uri);
		void showOperationError(const YSSCore::General::YSSURI& uri,
			const YSSCore::General::YSSFileError& error);
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
