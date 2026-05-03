#pragma once
#include <QtCore/qdir.h>
#include <QtWidgets/qwidget.h>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QTreeView;
class QFileSystemModel;
namespace YSS::Editor {
	class ResourceBrowser :public QWidget {
		Q_OBJECT;
	private:
		QTreeView* FileTree;
		QFileSystemModel* FileModel;
		QPushButton* RefreshButton;
		QPushButton* ShrinkButton;
		QPushButton* ExpandButton;
		QPushButton* NewButton;
		QWidget* ButtonWidget;
		QHBoxLayout* ButtonLayout;
		QVBoxLayout* Layout;
		QDir CurrentDir;
		QMenu* Menu;
		QAction* ActionExpand;
		QAction* ActionShrink;
		QAction* ActionRename;
		QAction* ActionShowInExplorer;
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		void openNewFileWindow();
		void refresh();
	public:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
	};
}