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
		QPushButton* NewButton;
		QWidget* ButtonWidget;
		QHBoxLayout* ButtonLayout;
		QVBoxLayout* Layout;
		QDir CurrentDir;
	public:
		ResourceBrowser(QWidget* parent = nullptr);
		void openNewFileWindow();
	public:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void onNewButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(const QModelIndex& index);
	};
}