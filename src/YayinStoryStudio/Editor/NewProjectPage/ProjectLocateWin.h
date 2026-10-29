#ifndef YayinStoryStudio_Editor_NewProjectPage_ProjectLocateWin_h
#define YayinStoryStudio_Editor_NewProjectPage_ProjectLocateWin_h
#include <General/YSSURI.h>
#include <QtWidgets/qdialog.h>
// Forward declarations
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QToolButton;
namespace YSS::Editor {
	class RemoteBrowserWidget;
}
namespace YSS::NewProjectPage {
	class ProjectLocateWin :public QDialog {
		Q_OBJECT;
	signals:
		void targetChosen(const YSSCore::General::YSSURI& projectUri);
	private:
		QLabel* TitleLabel;
		QLabel* SubtitleLabel;
		QLineEdit* NameEdit;
		QToolButton* LocalModeButton;
		QToolButton* RemoteModeButton;
		QStackedWidget* TargetStack;
		QLineEdit* LocalPathEdit;
		QPushButton* BrowseButton;
		YSS::Editor::RemoteBrowserWidget* RemoteBrowser;
		QLabel* WhereLabel;
		QPushButton* ConfirmButton;
		QPushButton* CancelButton;
	public:
		explicit ProjectLocateWin(QWidget* parent = nullptr);
		virtual ~ProjectLocateWin();
		void setProjectName(const QString& name);
		bool isRemoteMode() const;
		YSSCore::General::YSSURI getTargetFolder() const;
	protected:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void buildUi();
		void applyStyle();
		void onModeChanged();
		void onBrowseClicked();
		void updateWhereLabel();
		void onConfirmClicked();
	};
}
#endif // YayinStoryStudio_Editor_NewProjectPage_ProjectLocateWin_h
