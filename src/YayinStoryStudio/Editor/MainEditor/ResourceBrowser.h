#ifndef YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
#define YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtWidgets/qwidget.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
#include <QtWidgets/qabstractitemdelegate.h>
class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;
class QTreeView;
class QFileSystemModel;
class QToolBar;
class QUndoCommand;
namespace YSS::Editor {
	class ResourceBrowserVFS :public YSSCore::Editor::FileServer {
		Q_OBJECT;
	public:
		ResourceBrowserVFS(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class ResourceBrowser :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	private:
		QTreeView* FileTree;
		QFileSystemModel* FileModel;
		QVBoxLayout* Layout;
		QDir ProjectRootDir;
		QToolBar* ToolBar;
		QAction* ToolActionRefresh;
		QAction* ToolActionExpand;
		QAction* ToolActionShrink;
		QAction* ToolActionNewFile;
		QAction* ToolActionNewFolder;
		QColor TextColor;
		QMenu* FileOptions;
		QString CurrentFilePath;
		QPersistentModelIndex RightClickedIndex;
		QAction* FileOptionOpen;
		QAction* FileOptionRename;
		QAction* FileOptionDelete;
		QAction* FileOptionNewFile;
		QAction* FileOptionNewFolder;
		QAction* FileOptionCopyPath;
		QAction* FileOptionCopyName;
		QAction* FileOptionCopy;
		QAction* FileOptionPaste;
		QAction* FileOptionCut;
		QAction* FileOptionRefresh;
	signals:
		void visibilityChanged(bool visible);
		void otherFileRenamed(const QString& path, const QString& oldName, const QString& newName);
		void directoryRenamed(const QString& path, const QString& oldName, const QString& newName);
		void fileOperationRequested(QUndoCommand* command);
	public:
		static ResourceBrowser* getInstance();
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		virtual ~ResourceBrowser();
		void refresh();
		void setCurrentSelected(const QFileInfo& path);
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
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_ResourceBrowser_h
