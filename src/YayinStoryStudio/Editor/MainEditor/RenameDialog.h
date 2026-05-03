#ifndef YSSCore_Editor_RenameDialog_h
#define YSSCore_Editor_RenameDialog_h
#include <QtWidgets/qframe.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>

namespace YSS::Editor {
	class RenameDialog :public QFrame {
		Q_OBJECT;
	signals:
		void renameConfirmed(const QString& absOldPath, const QString& newName);
	public:
		RenameDialog();
		void setContext(const QString& absOldPath, const QString& newName = QString());
	private:
		void onNewNameChanged(const QString& newName);
	private:
		QString AbsOldPath;
		QLabel* TitleLabel;
		QLineEdit* Edit;
		QLabel* WarningLabel;
		QPushButton* ConfirmButton;
		QPushButton* CancelButton;
		QVBoxLayout* Layout;
		bool validNewName = false;
		bool dirMode = false;
	};
}
#endif // YSSCore_Editor_RenameDialog_h
