#ifndef YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_h
#define YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_h
#include <General/YSSURI.h>
#include <QtWidgets/qframe.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/BorderFrame.h>
class QLabel;
class QGridLayout;
class QPushButton;
class QDateTime;

namespace YSSCore::General {
	class YSSProject;
}
namespace YSS::ProjectPage {
	class ProjectInfoWidget :public Visindigo::Widgets::BorderFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void removeConfirmed(const YSSCore::General::YSSURI& uri);
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
		YSSCore::General::YSSURI CurrentUri;
		int SizeRequestId = 0;
	public:
		ProjectInfoWidget(QWidget* parent);
		void showProject(YSSCore::General::YSSProject* project);
		void showUri(const YSSCore::General::YSSURI& uri, const QString& name,
			const QDateTime& createTime, const QDateTime& lastModified);
		void initWidget();
		void onDeleteButtonClicked();
		void onShowInBrowserButtonClicked();
		void onRemoveFromListButtonClicked();
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void onThemeChanged() override;
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_ProjectInfoWidget_h
