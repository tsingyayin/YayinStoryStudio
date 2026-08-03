#ifndef YSS_Editor_InstallerClient_h
#define YSS_Editor_InstallerClient_h
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
	public:
		void syncProgramVersion();
	private:
		InstallerClientPrivate* d;
	};
}
#endif // YSS_Editor_InstallerClient_h
