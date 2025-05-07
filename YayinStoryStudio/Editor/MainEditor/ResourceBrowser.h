#pragma once
#include <QDir>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

namespace YSS::Editor {
	class ResourceBrowser :public QWidget {
		Q_OBJECT;
	private:
		QListWidget* FileList;
		QLineEdit* CurrentPath;
		QPushButton* RefreshButton;
		QPushButton* NewButton;
		QPushButton* BackButton;
		QWidget* ButtonWidget;
		QHBoxLayout* ButtonLayout;
		QVBoxLayout* Layout;

		QDir CurrentDir;
	public:
		ResourceBrowser(QWidget* parent = nullptr);
	private:
		void onBackButtonClicked();
		void refreshFileList();
		void onItemDoubleClicked(QListWidgetItem* item);
	};
}