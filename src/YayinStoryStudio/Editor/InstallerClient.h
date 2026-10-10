#ifndef YayinStoryStudio_Editor_InstallerClient_h
#define YayinStoryStudio_Editor_InstallerClient_h
#include <QtCore/qobject.h>
#include <QtNetwork/qlocalsocket.h>
#include <Utility/JsonConfig.h>
namespace YSS::Editor {
	class InstallerClientPrivate;
	class InstallerClient :public QObject {
		Q_OBJECT;
	signals:
		void connected();
		void disconnected();
		void installerNotLaunched();
		void installerRequestProgramClose();
	public:
		static InstallerClient* getInstance();
	public:
		InstallerClient(QObject* parent = nullptr);
		virtual ~InstallerClient();
		void connectToInstaller();
		void sendCommand(const Visindigo::Utility::JsonConfig& command);
		void requestCrashArchive();
		static void releaseInstaller(bool autoLaunch = false, bool showNotification = false);
	public:
		void syncProgramVersion();
	private:
		void requestInstallerClose();
	private:
		InstallerClientPrivate* d;
	};
}
#endif // YayinStoryStudio_Editor_InstallerClient_h
