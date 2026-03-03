#ifndef YSS_Editor_ProjectMetaWin_H
#define YSS_Editor_ProjectMetaWin_H
#include <QtWidgets/qframe.h>

namespace YSS::ProjectMetaPage {
	class ProjectMetaWin :public QFrame {
		Q_OBJECT;
	public:
		ProjectMetaWin(QWidget* parent = nullptr) {};
	};
}

#endif // YSS_Editor_ProjectMetaWin_H