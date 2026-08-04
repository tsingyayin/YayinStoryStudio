#ifndef YSS_Installer_LocalUpdateWizard_h
#define YSS_Installer_LocalUpdateWizard_h
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
		//! 用户选择将新版本作为独立版本使用（保留旧版，不升级）时发出。
		void asIndependent(const InstallerClientData& clientData);
	public:
		bool checkProgramStillRunning(const QList<InstallerClientData>& targets);
		bool updateProgram(const InstallerClientData& from, const QList<InstallerClientData>& targets);
		//! 更新过程结束后调用，展示“更新完成”页面。
		void onUpdateFinished();
	private:
		LocalUpdateWizardPrivate* d;
	};
}
#endif // YSS_Installer_LocalUpdateWizard_h
