#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

class QLabel;
class QLineEdit;
class QScrollArea;
class QGridLayout;
class QVBoxLayout;

namespace Visindigo::Widgets {
	class MultiButton;
}
namespace Visindigo::Editor {
	class FileTemplateProvider;
}
namespace YSS::NewFilePage {
	class NewFileWin :public QWidget {
		Q_OBJECT;
	signals:
		void filePrepared(QString projectPath);
	private:
		QLabel* TitleLabel;
		QLabel* RecentTemplateTitle;
		QLineEdit* SearchLineEdit;
		QList<Visindigo::Widgets::MultiButton*> FileList;
		QList<Visindigo::Widgets::MultiButton*> RecentList;
		QMap<Visindigo::Widgets::MultiButton*, Visindigo::Editor::FileTemplateProvider*> ProviderMap;
		QScrollArea* RecentTemplateArea;
		QWidget* RecentTemplateWidget;
		QVBoxLayout* RecentTemplateLayout;
		QScrollArea* FileTemplateArea;
		QWidget* FileTemplateWidget;
		QVBoxLayout* FileTemplateLayout;
		QGridLayout* Layout;
		QString InitPath;
	public:
		NewFileWin(const QString& initPath);
		~NewFileWin() {};
		void loadRecentTemplate() {};
		void loadFileTemplate();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onTemplateButtonClicked();
		void onTemplateInitWidgetClosed();
		void onFilePrepared(QString projectPath);
		void refreshTemplate() {};
	};
}