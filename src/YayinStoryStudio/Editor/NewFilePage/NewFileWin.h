#ifndef YayinStoryStudio_Editor_NewFilePage_NewFileWin_h
#define YayinStoryStudio_Editor_NewFilePage_NewFileWin_h
#include <General/YSSURI.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtWidgets/qwidget.h>
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
	class FileTemplateProvider;
}
namespace YSS::NewFilePage {
	class NewFileWin :public QWidget, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void filePrepared(const YSSCore::General::YSSURI& uri);
	private:
		Visindigo::Widgets::BorderLabel* TitleLabel;
		Visindigo::Widgets::BorderLabel* RecentTemplateTitle;
		QLineEdit* SearchLineEdit;
		QList<Visindigo::Widgets::MultiButton*> FileList;
		QList<Visindigo::Widgets::MultiButton*> RecentList;
		QMap<Visindigo::Widgets::MultiButton*, YSSCore::Editor::FileTemplateProvider*> ProviderMap;
		QScrollArea* RecentTemplateArea;
		QWidget* RecentTemplateWidget;
		QVBoxLayout* RecentTemplateLayout;
		QScrollArea* FileTemplateArea;
		QWidget* FileTemplateWidget;
		QVBoxLayout* FileTemplateLayout;
		QGridLayout* Layout;
		YSSCore::General::YSSURI InitUri;
	public:
		NewFileWin(const YSSCore::General::YSSURI& initUri);
		~NewFileWin() {};
		void loadRecentTemplate() {};
		void loadFileTemplate();
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		void createFile(const YSSCore::General::YSSURI& uri, const QByteArray& content);
		void reportCreateFailure(const QString& title, const QString& message);
	public slots:
		void onTemplateButtonClicked();
		void refreshTemplate() {};
	};
}
#endif // YayinStoryStudio_Editor_NewFilePage_NewFileWin_h
