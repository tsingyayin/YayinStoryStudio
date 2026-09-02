#ifndef YSSInstaller_Installer_LocalUpdateWizard_h
#define YSSInstaller_Installer_LocalUpdateWizard_h
#include <QtWidgets/qwidget.h>
#include <QtCore/qlist.h>
#include <Installer/InstallerClientData.h>
namespace YSS::Installer {
	class LocalUpdateWizardPrivate;
	class LocalUpdateWizard :public QWidget {
		Q_OBJECT;
	public:
		LocalUpdateWizard(const InstallerClientData& clientData, QWidget* parent = nullptr);
		virtual ~LocalUpdateWizard();
	signals:
		void asIndependent(const InstallerClientData& clientData);
	public:
		QList<bool> checkProgramStillRunning(const QList<InstallerClientData>& targets);
		bool updateProgram(const InstallerClientData& from, const QList<InstallerClientData>& targets);
		void onUpdateFinished();
		void onAutoLaunch();
	private:
		LocalUpdateWizardPrivate* d;
	};
}
#endif // YSSInstaller_Installer_LocalUpdateWizard_h
