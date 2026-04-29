#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
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
		void filePrepared(QString projectPath);
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
		QString InitPath;
	public:
		NewFileWin(const QString& initPath);
		~NewFileWin() {};
		void loadRecentTemplate() {};
		void loadFileTemplate();
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onTemplateButtonClicked();
		void onFilePrepared(QString projectPath);
		void refreshTemplate() {};
	};
}