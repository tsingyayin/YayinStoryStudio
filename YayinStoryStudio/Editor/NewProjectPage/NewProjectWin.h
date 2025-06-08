#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
class QLabel;
class QLineEdit;
class QScrollArea;

namespace YSSCore::Widgets {
	class MultiButton;
}
namespace YSS::NewProjectPage {
	class NewProjectWin :public QWidget {
		Q_OBJECT;
	private:
		QLabel* RecentTemplateTitle;
		QLineEdit* SearchLineEdit;
		QList<YSSCore::Widgets::MultiButton*> ProjectList;
		QList<YSSCore::Widgets::MultiButton*> RecentList;
		QScrollArea* RecentTemplateArea;
		QWidget* RecentTemplateWidget;
		QScrollArea* ProjectTemplateArea;
		QWidget* ProjectTemplateWidget;
	public:
		NewProjectWin() {};
		~NewProjectWin() {};
		void loadRecentTemplate() {};
	public slots:
		void refreshTemplate() {};
	};
}