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
		// 请求把该 URI 从历史列表里移除。本地项目也是先取 YSSProject 的 URI 再发出来，
		// 这样本地和远端走同一条路径。
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
		// 当前展示的项目 URI；远端未加载的项目没有 YSSProject，也能据此移除。
		YSSCore::General::YSSURI CurrentUri;
		int SizeRequestId = 0;
	public:
		ProjectInfoWidget(QWidget* parent);
		void showProject(YSSCore::General::YSSProject* project);
		// 没有 YSSProject 对象时（例如未连接的远端项目）按已知信息展示。
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
