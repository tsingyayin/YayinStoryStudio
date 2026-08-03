#ifndef YSS_Installer_InstallerClientData_h
#define YSS_Installer_InstallerClientData_h
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <General/Version.h>
namespace YSS::Installer {
	class InstallerClientData{
		Q_GADGET;
		Q_PROPERTY(QString programPath READ getProgramPath WRITE setProgramPath);
		Q_PROPERTY(QString programVersion READ getProgramVersion WRITE setProgramVersion);
		Q_PROPERTY(bool autoUpdateEnabled READ getAutoUpdateEnabled WRITE setAutoUpdateEnabled);
	public:
		QString getProgramVersion() const {
			return programVersion.toString();
		}
		void setProgramVersion(const QString& version) {
			programVersion.setVersion(version);
		}
		QString getProgramPath() const {
			return programPath;
		}
		void setProgramPath(const QString& path) {
			programPath = path;
		}
		bool getAutoUpdateEnabled() const {
			return autoUpdateEnabled;
		}
		void setAutoUpdateEnabled(bool enabled) {
			autoUpdateEnabled = enabled;
		}
	private:
		Visindigo::General::Version programVersion;
		QString programPath;
		bool autoUpdateEnabled;
	};
}
#endif // YSS_Installer_InstallerClientData_h
