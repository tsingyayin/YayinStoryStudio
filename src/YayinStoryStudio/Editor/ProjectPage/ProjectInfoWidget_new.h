#ifndef YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_new_h
#define YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_new_h
#include <QtWidgets/qframe.h>
#include <QtGui/qpixmap.h>
#include <Widgets/ThemeManager.h>
class QLabel;
class QGridLayout;
class QPushButton;

namespace YSSCore::General {
	class YSSProject;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget_new :public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void removeConfirmed(YSSCore::General::YSSProject* who);
	private:
		QLabel* CoverLabel;
		QLabel* TitleLabel;
		QLabel* SizeLabel;
		QLabel* CreateTimeLabel;
		QLabel* LastModifiedTimeLabel;
		QPushButton* ShowInBrowserButton;
		QPushButton* RemoveFromListButton;
		QPushButton* DeleteButton;
		QGridLayout* Layout;
		YSSCore::General::YSSProject* Project = nullptr;
		QPixmap CoverSource;
		int SizeRequestId = 0;
	public:
		ProjectInfoWidget_new(QWidget* parent);
		void showProject(YSSCore::General::YSSProject* project);
		void initWidget();
		void onDeleteButtonClicked();
		void onShowInBrowserButtonClicked();
		void onRemoveFromListButtonClicked();
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void onThemeChanged() override;
	private:
		void updateCoverPixmap();
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_new_h
