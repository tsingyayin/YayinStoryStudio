#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

class QLabel;
class QLineEdit;
class QScrollArea;
class QGridLayout;
class QVBoxLayout;

namespace YSSCore::Widgets {
	class MultiButton;
}
namespace YSSCore::Editor {
	class ProjectTemplateProvider;
}
namespace YSS::NewProjectPage {
	class NewProjectWin :public QWidget {
		Q_OBJECT;
	signals:
		void projectPrepared(QString projectPath);
	private:
		QLabel* TitleLabel;
		QLabel* RecentTemplateTitle;
		QLineEdit* SearchLineEdit;
		QList<YSSCore::Widgets::MultiButton*> ProjectList;
		QList<YSSCore::Widgets::MultiButton*> RecentList;
		QMap<YSSCore::Widgets::MultiButton*, YSSCore::Editor::ProjectTemplateProvider*> ProviderMap;
		QScrollArea* RecentTemplateArea;
		QWidget* RecentTemplateWidget;
		QVBoxLayout* RecentTemplateLayout;
		QScrollArea* ProjectTemplateArea;
		QWidget* ProjectTemplateWidget;
		QVBoxLayout* ProjectTemplateLayout;
		QGridLayout* Layout;
	public:
		NewProjectWin();
		~NewProjectWin() {};
		void loadRecentTemplate() {};
		void loadProjectTemplate();
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onTemplateButtonClicked();
		void onTemplateInitWidgetClosed();
		void onProjectPrepared(QString projectPath);
		void refreshTemplate() {};
	};
}