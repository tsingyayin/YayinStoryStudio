#pragma once
#include <QtCore/qdir.h>
#include <QtWidgets/qwidget.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/BorderFrame.h>
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
namespace YSS::Editor {
	class ResourceBrowser :public Visindigo::Widgets::BorderFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QTreeView* FileTree;
		QFileSystemModel* FileModel;
		QVBoxLayout* Layout;
		QDir CurrentDir;
		QMenu* Menu;
		QToolBar* ToolBar;
		QAction* ToolActionRefresh;
		QAction* ToolActionExpand;
		QAction* ToolActionShrink;
		QAction* ToolActionNew;
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
		virtual void onThemeChanged() override;
		virtual void paintEvent(QPaintEvent* event) override;
	private:
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
	};
}