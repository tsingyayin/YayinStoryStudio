#ifndef YayinStoryStudio_Editor_ProjectPage_OpenRemoteProjectWin_h
#define YayinStoryStudio_Editor_ProjectPage_OpenRemoteProjectWin_h
#include <General/YSSURI.h>
#include <QtWidgets/qdialog.h>
// Forward declarations
class QLabel;
class QPushButton;
namespace YSS::Editor {
	class RemoteBrowserWidget;
}
namespace YSS::ProjectPage {
	class OpenRemoteProjectWin :public QDialog {
		Q_OBJECT;
	signals:
		void projectChosen(const YSSCore::General::YSSURI& projectUri);
	private:
		QLabel* TitleLabel;
		QLabel* SubtitleLabel;
		YSS::Editor::RemoteBrowserWidget* RemoteBrowser;
		QLabel* SelectionLabel;
		QPushButton* OpenButton;
		QPushButton* CancelButton;
	public:
		explicit OpenRemoteProjectWin(QWidget* parent = nullptr);
		virtual ~OpenRemoteProjectWin();
	protected:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void buildUi();
		void applyStyle();
		void updateSelectionLabel();
		void chooseProject(const YSSCore::General::YSSURI& uri);
		void onOpenClicked();
	};
}
#endif // YayinStoryStudio_Editor_ProjectPage_OpenRemoteProjectWin_h
