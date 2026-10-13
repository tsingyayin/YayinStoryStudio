#ifndef YSSInstaller_Installer_PSOD_h
#define YSSInstaller_Installer_PSOD_h
#include <QtCore/qstring.h>
#include <QtWidgets/qwidget.h>
namespace YSS::Installer {
	class PSODPrivate;
	class PSOD :public QWidget {
		Q_OBJECT;
	signals:
		void dismissed();
	public:
		PSOD(const QString& reportPath, const QString& reportFolder, QWidget* parent = nullptr);
		virtual ~PSOD();
		void execute();
	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
	private:
		PSODPrivate* d;
	};
}
#endif // YSSInstaller_Installer_PSOD_h
