#pragma once
#include <QtCore/qdir.h>
#include <QtWidgets/qwidget.h>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class QToolButton;
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
		QToolButton* RefreshButton;
		QToolButton* ShrinkButton;
		QToolButton* ExpandButton;
		QToolButton* NewButton;
		QWidget* ButtonWidget;
		QHBoxLayout* ButtonLayout;
		QVBoxLayout* Layout;
		QDir CurrentDir;
		QMenu* Menu;
		QAction* ActionExpand;
		QAction* ActionShrink;
		QAction* ActionRename;
		QAction* ActionShowInExplorer;
	signals:
		void visibilityChanged(bool visible);
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		void openNewFileWindow();
		void refresh();
	public:
		virtual void showEvent(QShowEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
	};
}