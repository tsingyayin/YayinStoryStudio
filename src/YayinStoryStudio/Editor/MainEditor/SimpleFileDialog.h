#ifndef YayinStoryStudio_Editor_MainEditor_SimpleFileDialog_h
#define YayinStoryStudio_Editor_MainEditor_SimpleFileDialog_h
#include <QtWidgets/qframe.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtCore/qstringlist.h>

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

	class NewFolderDialog :public QFrame {
		Q_OBJECT;
	signals:
		void confirmed(const QString& newName);
	public:
		NewFolderDialog();
		void setContext(const QString& parentPath, const QStringList& existingNames);
	private:
		void validateName();
		void onConfirmClicked();
	private:
		QString ParentPath;
		QStringList ExistingNames;
		QLabel* DescriptionLabel;
		QLineEdit* NameEdit;
		QLabel* CheckLabel;
		QPushButton* ConfirmButton;
		QPushButton* CancelButton;
		QVBoxLayout* MainLayout;
		bool ValidName = false;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_SimpleFileDialog_h
