#ifndef YSS_MainEditor_MainWinMenu_h
#define YSS_MainEditor_MainWinMenu_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qaction.h>
namespace YSS::Editor {
	class MainWin;
	class MainWinMenuPrivate;
	class MainWinMenu :public QFrame {
		Q_OBJECT;
	public:
		MainWinMenu(MainWin* parent);
		virtual ~MainWinMenu();
	private:
		MainWinMenuPrivate* d;
	};
}
#endif // YSS_MainEditor_MainWinMenu_h