#ifndef YayinStoryStudio_Editor_MainEditor_SimpleFileDialog_h
#define YayinStoryStudio_Editor_MainEditor_SimpleFileDialog_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qdialog.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtCore/qstringlist.h>
#include <General/YSSURI.h>

namespace YSS::Editor {
	class RenameDialog :public QDialog {
		Q_OBJECT;
	signals:
		void renameConfirmed(const YSSCore::General::YSSURI& oldUri,
			const YSSCore::General::YSSURI& newUri, bool isDirectory);
	public:
		RenameDialog();
		void setContext(const YSSCore::General::YSSURI& uri, const QString& newName = QString(),
			bool isDirectory = false);
	protected:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void applyStyle();
		void setWarning(const QString& text, bool error);
		void setConfirmEnabled(bool enabled);
		void onNewNameChanged(const QString& newName);
		YSSCore::General::YSSURI targetUri(const QString& newName) const;
	private:
		YSSCore::General::YSSURI OldUri;
		QLabel* TitleLabel;
		QLabel* SubtitleLabel;
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
		void setContext(const YSSCore::General::YSSURI& parentUri, const QStringList& existingNames);
	private:
		void validateName();
		void onConfirmClicked();
	private:
		YSSCore::General::YSSURI ParentUri;
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
