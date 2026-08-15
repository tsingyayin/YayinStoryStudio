#ifndef YSS_Editor_MainWin_ColorThemeSettings_p_h
#define YSS_Editor_MainWin_ColorThemeSettings_p_h
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtCore/qstringlist.h>
namespace YSS::Editor {
	class ColorThemeSettingsCopyDialog :public QWidget {
		Q_OBJECT;
	signals:
		void confirmed(const QString& newName);
	public:
		ColorThemeSettingsCopyDialog(QWidget* parent = nullptr);
		~ColorThemeSettingsCopyDialog();
	public:
		void setCurrentThemes(const QStringList& themes);
		void setFromThemeName(const QString& name);
	private:
		QLabel* DescriptionLabel;
		QLineEdit* NameEdit;
		QLabel* CheckLabel;
		QPushButton* ConfirmButton;
		QPushButton* CancelButton;
		QVBoxLayout* MainLayout;
		QStringList CurrentThemes;
		QString FromThemeName;
		bool ValidName = false;
		void validateName();
		void onConfirmClicked();
	};
}
#endif // YSS_Editor_MainWin_ColorThemeSettings_p_h
