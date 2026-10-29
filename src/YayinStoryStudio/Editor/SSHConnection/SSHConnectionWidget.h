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
	// SSH 连接管理页。为 YSSUniqueFileSystem 提供登录与连接管理：
	// 新增连接、查看已有连接、断开连接，以及在首次连接时由用户确认服务端指纹。
	//
	// 凭据不落盘：密码每次输入，密钥文件也每次手动定位。页面只持久化两样东西，
	// 都写在主插件配置里：
	// - SSH.Authentics：最近使用过的 user@host:port，字符串数组。
	// - SSH.HostKeys：已确认过的服务端指纹，对象数组，含 Authority / KeyType /
	//   FingerprintSha256。后续连接自动比对该记录，不再重复询问。
	//
	// 凭据不落盘，所以重启程序后 YSSUniqueFileSystem 里一条连接都没有。下方
	// “已登记的连接”列表因此不是只映射 UFS 的运行时状态，而是把 UFS 里本次会话
	// 登记过的连接、SSH.Authentics 和 SSH.HostKeys 合并展示，未连接不等于不存在。
	class SSHConnectionWidget : public QDialog {
		Q_OBJECT;
	signals:
		// 某条连接已经建立，并且主机密钥检查通过。
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
		// 正在尝试连接的 authority。为空表示没有进行中的尝试。
		QString PendingAuthority;
		// 正在等待用户确认指纹的 authority。非空时忽略该连接后续的成功通知。
		QString AwaitingFingerprint;
		// 待确认的主机密钥信息，用户点“信任”时写入配置。
		Visindigo::Network::SshHostKeyInfo PendingHostKey;
	public:
		static SSHConnectionWidget* getInstance();
		// 缺少远程连接配置时调用。uri 必须是 SSH 方案：此时会显示本页并切到新建
		// 连接状态，用 uri 里的用户、主机、端口填好 authentic。其他方案不做任何事。
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
		// 按当前配色方案重建本页样式（卡片、分段控件、主按钮等）。
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
		// 把页面切到"为这条连接输入凭据"的状态并显示出来。requestVerificationFor()
		// 与 UFS 的 sshConnectionRequired() 都走这里。
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
		// UFS 报告某条连接当前不可用、需要凭据。
		void onConnectionRequired(const QString& authority);
	private:
		// SSH.Authentics 的读写。
		static QStringList loadAuthentics();
		static void rememberAuthentic(const QString& authority);
		static void forgetAuthentic(const QString& authority);
		// SSH.HostKeys 的读写。找不到记录时返回空字符串。
		static QString findStoredFingerprint(const QString& authority);
		static QStringList loadKnownHosts();
		static void storeFingerprint(const QString& authority,
			const Visindigo::Network::SshHostKeyInfo& info);
		static void forgetFingerprint(const QString& authority);
		// 把 user@host:port 拆成三段；缺端口时按 22 处理。
		static bool parseAuthority(const QString& authority, QString* user, QString* host,
			qint32* port);
	};
}
#endif // YayinStoryStudio_Editor_SSHConnection_SSHConnectionWidget_h
