#pragma once
#include <QtCore/qdir.h>
#include <QtWidgets/qwidget.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/BorderFrame.h>
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
	class ResourceBrowser :public Visindigo::Widgets::BorderFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QTreeView* FileTree;
		QFileSystemModel* FileModel;
		QVBoxLayout* Layout;
		QDir CurrentDir;
		QToolBar* ToolBar;
		QAction* ToolActionRefresh;
		QAction* ToolActionExpand;
		QAction* ToolActionShrink;
		QAction* ToolActionNew;
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
	signals:
		void visibilityChanged(bool visible);
		void fileRenamed(const QString& path, const QString& oldName, const QString& newName);
		void fileOperationRequested(QUndoCommand* command);
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		void openNewFileWindow();
		void refresh();
	public:
		virtual void showEvent(QShowEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void onThemeChanged() override;
		virtual void paintEvent(QPaintEvent* event) override;
	private:
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
		void onFileTreeContextMenuRequested(const QPoint& pos);
		void onRenameTriggered();
	};
}