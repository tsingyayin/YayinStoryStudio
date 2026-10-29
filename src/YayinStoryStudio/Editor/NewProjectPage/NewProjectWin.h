#ifndef YayinStoryStudio_Editor_NewProjectPage_NewProjectWin_h
#define YayinStoryStudio_Editor_NewProjectPage_NewProjectWin_h
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <General/YSSURI.h>
#include <Widgets/ThemeManager.h>

class QLabel;
class QLineEdit;
class QScrollArea;
class QGridLayout;
class QVBoxLayout;

namespace Visindigo::Widgets {
	class MultiButton;
	class BorderLabel;
}
namespace YSSCore::Editor {
	class ProjectTemplateProvider;
}
namespace YSS::NewProjectPage {
	class ProjectLocateWin;
	class NewProjectWin :public QWidget, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void projectPrepared(const YSSCore::General::YSSURI& projectUri);
	private:
		Visindigo::Widgets::BorderLabel* TitleLabel;
		Visindigo::Widgets::BorderLabel* RecentTemplateTitle;
		QLineEdit* SearchLineEdit;
		QList<Visindigo::Widgets::MultiButton*> ProjectList;
		QList<Visindigo::Widgets::MultiButton*> RecentList;
		QMap<Visindigo::Widgets::MultiButton*, YSSCore::Editor::ProjectTemplateProvider*> ProviderMap;
		QScrollArea* RecentTemplateArea;
		QWidget* RecentTemplateWidget;
		QVBoxLayout* RecentTemplateLayout;
		QScrollArea* ProjectTemplateArea;
		QWidget* ProjectTemplateWidget;
		QVBoxLayout* ProjectTemplateLayout;
		QGridLayout* Layout;
		YSSCore::Editor::ProjectTemplateProvider* PendingProvider = nullptr;
	public:
		NewProjectWin();
		~NewProjectWin() {};
		void loadRecentTemplate() {};
		void loadProjectTemplate();
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onTemplateButtonClicked();
		void onLocateTargetChosen(const YSSCore::General::YSSURI& projectUri);
		void onProjectPrepared();
		void refreshTemplate() {};
	};
}
#endif // YayinStoryStudio_Editor_NewProjectPage_NewProjectWin_h
