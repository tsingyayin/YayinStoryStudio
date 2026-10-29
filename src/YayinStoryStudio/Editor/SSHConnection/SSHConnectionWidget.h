#ifndef YayinStoryStudio_Editor_SSHConnection_SSHConnectionWidget_h
#define YayinStoryStudio_Editor_SSHConnection_SSHConnectionWidget_h
#include <General/YSSURI.h>
#include <Network/SshTypes.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qdialog.h>
// Forward declarations
class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QToolButton;
class QVBoxLayout;
class QWidget;
namespace YSS::Editor {
	class SSHConnectionWidget : public QDialog {
		Q_OBJECT;
	signals:
		void connectionReady(const QString& authority);
	private:
		QLabel* TitleLabel;
		QLabel* SubtitleLabel;
		QComboBox* AuthenticBox;
		QToolButton* PasswordModeButton;
		QToolButton* KeyModeButton;
		QStackedWidget* CredentialStack;
		QLineEdit* PasswordEdit;
		QLineEdit* KeyPathEdit;
		QPushButton* BrowseKeyButton;
		QPushButton* ConnectButton;
		QLabel* StatusLabel;
		QListWidget* ConnectionList;
		QPushButton* DisconnectButton;
		QPushButton* RemoveButton;
		QPushButton* RefreshButton;
		QFrame* FingerprintPanel;
		QLabel* FingerprintLabel;
		QPushButton* TrustFingerprintButton;
		QPushButton* RejectFingerprintButton;
		QVBoxLayout* Layout;
		QString PendingAuthority;
		QString AwaitingFingerprint;
		Visindigo::Network::SshHostKeyInfo PendingHostKey;
	public:
		static SSHConnectionWidget* getInstance();
		static void requestVerification(const YSSCore::General::YSSURI& uri);
	public:
		explicit SSHConnectionWidget(QWidget* parent = nullptr);
		virtual ~SSHConnectionWidget();
		virtual void showEvent(QShowEvent* event) override;
	public:
		void requestVerificationFor(const YSSCore::General::YSSURI& uri);
		void refreshConnectionList();
	private:
		void buildUi();
		void applyStyle();
		void setStatus(const QString& text, bool error = false);
		void setBusy(bool busy);
		void refreshAuthenticBox();
		QWidget* createConnectionRow(const QString& authority, bool ready) const;
		QWidget* createEmptyRow() const;
		void showFingerprintPrompt(const QString& authority,
			const Visindigo::Network::SshHostKeyInfo& info);
		void hideFingerprintPrompt();
		void connectTo(const QString& authority);
		QString currentAuthentic() const;
	private:
		void promptForAuthority(const QString& authority);
		void onConnectClicked();
		void onBrowseKeyClicked();
		void onCredentialModeChanged();
		void onDisconnectClicked();
		void onRemoveClicked();
		void onTrustFingerprintClicked();
		void onRejectFingerprintClicked();
		void onHostKeyAccepted(const QString& authority,
			const Visindigo::Network::SshHostKeyInfo& info);
		void onHostKeyRejected(const QString& authority,
			const Visindigo::Network::SshHostKeyInfo& info);
		void onConnectFinished(const QString& authority, bool ok, const QString& message);
		void onSshReadyChanged(const QString& authority, bool ready);
		void onConnectionRequired(const QString& authority);
	private:
		static QStringList loadAuthentics();
		static void rememberAuthentic(const QString& authority);
		static void forgetAuthentic(const QString& authority);
		static QString findStoredFingerprint(const QString& authority);
		static QStringList loadKnownHosts();
		static void storeFingerprint(const QString& authority,
			const Visindigo::Network::SshHostKeyInfo& info);
		static void forgetFingerprint(const QString& authority);
		static bool parseAuthority(const QString& authority, QString* user, QString* host,
			qint32* port);
	};
}
#endif // YayinStoryStudio_Editor_SSHConnection_SSHConnectionWidget_h
