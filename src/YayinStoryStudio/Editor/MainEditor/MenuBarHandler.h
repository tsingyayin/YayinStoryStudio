#pragma once
#include <QtCore/qobject.h>
#include <General/Log.h>
#include <Widgets/QuickMenu.h>

namespace YSS::Editor {
	class Menu_File_FileOptions :public QObject {
		Q_OBJECT;
		HandlerNode(Menu_File_FileOptions, "fileOptions");
		SubNodes{};
	public slots:
		void newfile();
		void open();
		void save();
		void saveAll();
		void saveAs();
	};
	class Menu_File_ProgramOptions :public QObject {
		Q_OBJECT;
		HandlerNode(Menu_File_ProgramOptions, "programOptions");
		SubNodes{};
	public slots:
		void backToHome();
		void preferences() {
			yDebug << "Preferences";
		}
		void exit();
	};
	class Menu_File :public QObject {
		Q_OBJECT;
		HandlerNode(Menu_File, "file");
		SubNodes{
			new Menu_File_FileOptions(this);
			new Menu_File_ProgramOptions(this);
		}
	};
	class Menu_Edit_EditOptions : public QObject {
		Q_OBJECT;
		HandlerNode(Menu_Edit_EditOptions, "editOptions");
		SubNodes{};
	public slots:
		void undo();
		void redo();
		void cut();
		void copy();
		void paste();
		void selectAll();
	};
	class Menu_Edit :public QObject {
		Q_OBJECT;
		HandlerNode(Menu_Edit, "edit");
		SubNodes{
			new Menu_Edit_EditOptions(this);
		};
	};
	class MenuActionHandler :public QObject {
		Q_OBJECT;
	public:
		MenuActionHandler(QObject* parent = nullptr) {
			new Menu_File(this);
			new Menu_Edit(this);
		}
	};
}