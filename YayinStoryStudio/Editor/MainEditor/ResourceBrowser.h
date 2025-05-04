#pragma once
#include <QtWidgets>

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