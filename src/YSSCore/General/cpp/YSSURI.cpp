#include "General/YSSURI.h"
#include "Editor/VirtualFilePath.h"
#include <Network/SshPath.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <variant>

namespace {
	using YSSCore::General::YSSURI;

	const QString& resourceSchemePrefix() {
		static const QString prefix = QStringLiteral("qrc:");
		return prefix;
	}

	bool isResourceText(const QString& text) {
		return text.startsWith(resourceSchemePrefix(), Qt::CaseSensitive) or text.startsWith(QChar(':'));
	}

	// Qt 的 QFile/QResource 同时接受 qrc:/x 与 :/x；它的规范形式是 :/x，
	// QFileInfo(":/x").absoluteFilePath() 也原样返回 :/x。返回值统一成这个规范形式，
	// 空字符串表示这不是一个可用的资源路径。
	QString resourcePathOf(const QString& text) {
		QString pathText = text;
		if (pathText.startsWith(resourceSchemePrefix(), Qt::CaseSensitive)) {
			pathText = pathText.mid(resourceSchemePrefix().size());
		}
		else if (pathText.startsWith(QChar(':'))) {
			pathText = pathText.mid(1);
		}
		else {
			return QString();
		}
		if (not pathText.startsWith(QChar('/'))) {
			return QString();
		}
		pathText = QDir::cleanPath(pathText);
		// cleanPath 不会把越过根的 .. 钳制掉，而这种路径在 Qt 资源里不可能存在。
		if (pathText == QStringLiteral("/..") or pathText.startsWith(QStringLiteral("/../"))) {
			return QString();
		}
		return QStringLiteral(":") + pathText;
	}

	// 本机路径是否区分大小写。Windows 的盘符与 UNC 路径不区分，其余平台区分。
	bool localPathsAreCaseSensitive() {
	#ifdef Q_OS_WIN
		return false;
	#else
		return true;
	#endif
	}

	Qt::CaseSensitivity localCaseSensitivity() {
		return localPathsAreCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	}
}

namespace YSSCore::General {
	class YSSURIPrivate {
		friend class YSSURI;
	protected:
		using Storage = std::variant<std::monostate, QFileInfo,
			Visindigo::Network::SshPath, YSSCore::Editor::VirtualFilePath, QString>;
	protected:
		YSSURI::Scheme SchemeType = YSSURI::Scheme::Unknown;
		Storage Value;
	protected:
		const QFileInfo* local() const {
			return std::get_if<QFileInfo>(&Value);
		}
		const Visindigo::Network::SshPath* sshPath() const {
			return std::get_if<Visindigo::Network::SshPath>(&Value);
		}
		const YSSCore::Editor::VirtualFilePath* virtualPath() const {
			return std::get_if<YSSCore::Editor::VirtualFilePath>(&Value);
		}
		const QString* resourcePath() const {
			return std::get_if<QString>(&Value);
		}
		QString resourcePathText() const {
			const QString* path = this->resourcePath();
			return path ? *path : QString();
		}
	};

	/*!
		\class YSSCore::General::YSSURI
		\brief 对本地资源、远端资源、虚拟文档和QRC资源的统一标识。
		\since YSS 0.18.0
		\inmodule YSSCore

		自0.18.0起，为了使YSS能支持SSH，引入了YSSURI+UniqueFileSystem类以统一标识
		本机资源、远程资源、虚拟文档和QRC资源。此YSSURI类用于标识资源的身份，并且
		提供良好的比较和哈希行为。

		\section1 设计理念
		总的来说，这个类不旨在创建一种统一的资源标识格式，只是将四种不同类型的资源路径
		统一封装在一个类里，因此实际使用的过程中，基本上要根据scheme的结果来使用
		对应的函数。

		\section1 统一规则
		话虽如此，为了统一不同平台的体验，在本地路径和相对路径问题上，YSSURI遵循以下规则：
		\list
		\li 1. 本地路径全部使用正斜杠作文路径分隔符。Windows上的反斜杠会被自动转换为正斜杠
		\li 2. 相对路径在存储时会被转换为绝对路径。在本地路径模式下，行为与QFileInfo::absoluteFilePath()一致。
		在远端路径下，如果相对路径超出了远程根，则会判定为无效的URI，而不是就地钳制到根。
		\li 3. 对于本地路径，Windwos平台不区分大小写，其他平台、远端路径、虚拟路径始终区分大小写。
		\li 4. QRC资源路径使用方式和Qt官方一致，支持qrc:/x和:/x两种写法。
		\endlist
		
		\section1 YSSURI下的文件读写
		YSSURI本身并不不负责读写。将YSSURI传递给UniqueFileSystem类的相关函数，才能真正读写文件。

		自0.18.0引入YSSURI后，YSS将具有原生的操作SSH远程文件的能力，因此建议下游使用YSSURI来标识文件，
		而不是直接使用QString或QFileInfo（除非解析后是本地资源文件）。
	*/

	/*!
		\enum YSSCore::General::YSSURI::Scheme
		\since YSS 0.18.0

		标识当前URI对应的资源类型。

		\value Unknown 无效URI。
		\value Local 本地文件。
		\value SSH SSH远端文件。
		\value VirtualFile 虚拟文档。
		\value Resource Qt资源。

		Qt资源是指由QRC文件定义的，编译进二进制的资源文件。详情请参考Qt官方文档。
	*/

	/*!
		默认构造函数

		通过此函数创建的URI将是一个无效的URI。

		\since YSS 0.18.0
	*/
	YSSURI::YSSURI() {
		this->d = new YSSURIPrivate();
	}

	/*!
		\fn YSSCore::General::YSSURI::YSSURI(YSSURI&& other) noexcept
		\since YSS 0.18.0
		移动构造函数。
	*/

	/*!
		\fn YSSCore::General::YSSURI& YSSCore::General::YSSURI::operator=(YSSURI&& other) noexcept
		\since YSS 0.18.0
		移动赋值运算符。

		\a other 要移动的源对象。
	*/
	VIMoveable_Impl(YSSURI);

	/*!
		\fn YSSCore::General::YSSURI::YSSURI(const YSSURI& other)
		\since YSS 0.18.0
		复制构造函数。
	*/

	/*!
		\fn YSSCore::General::YSSURI& YSSCore::General::YSSURI::operator=(const YSSURI& other)
		\since YSS 0.18.0
		复制赋值运算符。

		\a other 要复制的源对象。
	*/
	VICopyable_Impl(YSSURI);

	/*!
		\since YSS 0.18.0
		析构函数。
	*/
	YSSURI::~YSSURI() {
		delete d;
	}

	// 接管一份已经解析好的私有数据，工厂函数内部使用。
	YSSURI::YSSURI(YSSURIPrivate* data) {
		this->d = data;
	}

	/*!
		\since YSS 0.18.0
		\a path 本机路径，可以是绝对路径或相对路径，也可以直接是虚拟文档路径或QRC资源路径。
		\return 对应的YSSURI；无法解析时返回无效URI。

		本机路径交给 \l{QFileInfo::absoluteFilePath()}：相对路径按当前工作目录转成绝对路径，
		Windows 上的反斜杠一并转成正斜杠，随后原样存储这个绝对路径。虚拟文档与 QRC 资源
		不经过 QFileInfo。
	*/
	YSSURI YSSURI::fromLocalPath(const QString& path) {
		const QString text = path.trimmed();
		if (text.isEmpty()) {
			return YSSURI();
		}
		YSSURIPrivate* data = new YSSURIPrivate();
		if (YSSCore::Editor::VirtualFilePath::isVirtualFilePath(text)) {
			data->SchemeType = Scheme::VirtualFile;
			data->Value = YSSCore::Editor::VirtualFilePath(text);
			return YSSURI(data);
		}
		if (isResourceText(text)) {
			const QString resourcePath = resourcePathOf(text);
			if (resourcePath.isEmpty()) {
				delete data;
				return YSSURI();
			}
			data->SchemeType = Scheme::Resource;
			data->Value = resourcePath;
			return YSSURI(data);
		}
		data->SchemeType = Scheme::Local;
		data->Value = QFileInfo(QFileInfo(text).absoluteFilePath());
		return YSSURI(data);
	}

	/*!
		\since YSS 0.18.0
		\a text 任意形式的资源标识：虚拟文档路径、\c{ssh://} 远端路径、QRC 资源路径，或普通本机路径。
		\return 解析结果；无法识别时返回无效URI。

		识别顺序固定为虚拟文档、远端路径、QRC 资源、本机路径。本机路径不再接受任何
		自定义的 scheme 前缀，原样交给 \l{QFileInfo}。
	*/
	YSSURI YSSURI::fromString(const QString& text) {
		const QString trimmed = text.trimmed();
		if (trimmed.isEmpty()) {
			return YSSURI();
		}
		if (YSSCore::Editor::VirtualFilePath::isVirtualFilePath(trimmed)) {
			return fromVirtualPath(trimmed);
		}
		if (Visindigo::Network::SshPath::isSshPath(trimmed)) {
			const Visindigo::Network::SshPath sshPath = Visindigo::Network::SshPath::fromString(trimmed);
			if (not sshPath.isValid()) {
				return YSSURI();
			}
			YSSURIPrivate* data = new YSSURIPrivate();
			data->SchemeType = Scheme::SSH;
			data->Value = sshPath;
			return YSSURI(data);
		}
		return fromLocalPath(trimmed);
	}

	/*!
		\fn YSSCore::General::YSSURI YSSCore::General::YSSURI::fromVirtualPath(const QString& virtualPath)
		\since YSS 0.18.0
		\a virtualPath 形如 \c{@ext!name?param} 的虚拟文档路径。
		\return 对应的YSSURI；格式不合法时返回无效URI。
	*/
	YSSURI YSSURI::fromVirtualPath(const QString& virtualPath) {
		const QString trimmed = virtualPath.trimmed();
		if (not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(trimmed)) {
			return YSSURI();
		}
		YSSURIPrivate* data = new YSSURIPrivate();
		data->SchemeType = Scheme::VirtualFile;
		data->Value = YSSCore::Editor::VirtualFilePath(trimmed);
		return YSSURI(data);
	}

	/*!
		\since YSS 0.18.0
		\a user SSH用户名，可为空。
		\a host 主机名或带方括号的 IPv6 字面量，不能带端口。
		\a port 端口，小于等于0表示默认端口。
		\a path 远端绝对路径。
		\return 对应的YSSURI；主机名为空或路径越界时返回无效URI。
	*/
	YSSURI YSSURI::fromSsh(const QString& user, const QString& host, qint32 port, const QString& path) {
		const Visindigo::Network::SshPath sshPath = Visindigo::Network::SshPath::fromParts(user, host, port, path);
		if (not sshPath.isValid()) {
			return YSSURI();
		}
		YSSURIPrivate* data = new YSSURIPrivate();
		data->SchemeType = Scheme::SSH;
		data->Value = sshPath;
		return YSSURI(data);
	}

	/*!
		\since YSS 0.18.0
		\return 是否是一个可用的URI。
	*/
	bool YSSURI::isValid() const {
		return d->SchemeType != Scheme::Unknown;
	}

	/*!
		\since YSS 0.18.0
		\return 该URI的scheme。
	*/
	YSSURI::Scheme YSSURI::getScheme() const {
		return d->SchemeType;
	}

	/*!
		\since YSS 0.18.0
		\return 是否是本机文件，即 scheme 为 \l{Scheme::Local}。
		请注意，虽然QRC文件肯定也只能通过本地读取，但它属于 \l{Scheme::Resource}，
		不在此函数的判断范围内。

		如果要判断从本地读取的文件，应该使用 \l{isNotRemote()}。
	*/
	bool YSSURI::isLocal() const {
		return this->getScheme() == Scheme::Local;
	}

	/*!
		\since YSS 0.18.0
		\return 是否指向 Qt 资源（qrc）。
	*/
	bool YSSURI::isResource() const {
		return this->getScheme() == Scheme::Resource;
	}

	/*!
		\since YSS 0.18.0
		\return 是否指向SSH远端文件。
	*/
	bool YSSURI::isRemote() const {
		return this->getScheme() == Scheme::SSH;
	}

	/*!
		\since YSS 0.18.0
		\return 是否由本机后端处理，即 \l{Scheme::Local} 或 \l{Scheme::Resource}
	*/
	bool YSSURI::isNotRemote() const {
		const Scheme scheme = this->getScheme();
		return scheme == Scheme::Local or scheme == Scheme::Resource;
	}

	/*!
		\since YSS 0.18.0
		\return 是否指向虚拟文档。
	*/
	bool YSSURI::isVirtual() const {
		return this->getScheme() == Scheme::VirtualFile;
	}

	/*!
		\since YSS 0.18.0
		\return 是否指向一个存在的普通文件。

		只有 \l{Scheme::Local} 会真的查询文件系统（\l{QFileInfo::isFile()}）；远端、
		虚拟文档与 QRC 资源没有同步的文件系统信息，恒为 false。目录请用 \l{isDir()}。
	*/
	bool YSSURI::isFile() const {
		const QFileInfo* info = d->local();
		return info != nullptr and info->isFile();
	}

	/*!
		\since YSS 0.18.0
		\return 是否指向一个存在的目录。

		只有 \l{Scheme::Local} 会真的查询文件系统（\l{QFileInfo::isDir()}）；远端、
		虚拟文档与 QRC 资源恒为 false。文件请用 \l{isFile()}。
	*/
	bool YSSURI::isDir() const {
		const QFileInfo* info = d->local();
		return info != nullptr and info->isDir();
	}

	/*!
		\since YSS 0.18.0
		\return 本机文件信息；scheme 不是 \l{Scheme::Local} 时返回 \c{std::nullopt}。
	*/
	std::optional<QFileInfo> YSSURI::getLocalFileInfo() const {
		const QFileInfo* info = d->local();
		if (info == nullptr) {
			return std::nullopt;
		}
		return *info;
	}

	/*!
		\since YSS 0.18.0
		\return 远端路径；scheme 不是 \l{Scheme::SSH} 时返回 \c{std::nullopt}。
	*/
	std::optional<Visindigo::Network::SshPath> YSSURI::getSshPath() const {
		const Visindigo::Network::SshPath* path = d->sshPath();
		if (path == nullptr) {
			return std::nullopt;
		}
		return *path;
	}

	/*!
		\since YSS 0.18.0
		\return 虚拟文档路径；scheme 不是 \l{Scheme::VirtualFile} 时返回 \c{std::nullopt}。
	*/
	std::optional<YSSCore::Editor::VirtualFilePath> YSSURI::getVirtualFilePath() const {
		const YSSCore::Editor::VirtualFilePath* path = d->virtualPath();
		if (path == nullptr) {
			return std::nullopt;
		}
		return *path;
	}

	/*!
		\since YSS 0.18.0
		\return QRC 资源的 \c{:} 前缀形式（与 \l{QFile} 接受的形式一致，形如 \c{:/x}）；
		其余scheme返回空字符串。

		Qt 把资源路径写成 \c{:/x}，把这个字符串直接交给 \l{QFile} 就能打开。
	*/
	QString YSSURI::getResourcePath() const {
		return d->resourcePathText();
	}

	/*!
		\since YSS 0.18.0
		\return SSH用户名；其余scheme返回空字符串。
	*/
	QString YSSURI::getUser() const {
		const Visindigo::Network::SshPath* path = d->sshPath();
		return path ? path->getUser() : QString();
	}

	/*!
		\since YSS 0.18.0
		\return 虚拟文档的扩展名（FileServer ID），或 SSH 主机名；其余scheme返回空字符串。

		本机路径没有主机概念：\l{QFileInfo} 不区分 UNC 的主机名与共享名，因此这里也
		不额外拆分。
	*/
	QString YSSURI::getHost() const {
		if (const YSSCore::Editor::VirtualFilePath* path = d->virtualPath()) {
			return path->getExt();
		}
		if (const Visindigo::Network::SshPath* path = d->sshPath()) {
			return path->getHost();
		}
		return QString();
	}

	/*!
		\since YSS 0.18.0
		\return SSH显式端口；未指定或其余scheme返回-1。

		需要"实际使用哪个端口"时请改用 \l{Visindigo::Network::SshPath::getEffectivePort()}。
	*/
	qint32 YSSURI::getPort() const {
		const Visindigo::Network::SshPath* path = d->sshPath();
		return path ? path->getPort() : -1;
	}

	/*!
		\since YSS 0.18.0
		\return 规范化的路径：本机为绝对路径（正斜杠），远端为 POSIX 绝对路径，
		虚拟文档为名称部分，QRC 为 \c{:} 前缀的资源路径（\c{:/x}）。
	*/
	QString YSSURI::getPath() const {
		if (const QFileInfo* info = d->local()) {
			return info->absoluteFilePath();
		}
		if (const Visindigo::Network::SshPath* path = d->sshPath()) {
			return path->getPath();
		}
		if (const YSSCore::Editor::VirtualFilePath* path = d->virtualPath()) {
			return path->getFileName();
		}
		return d->resourcePathText();
	}

	/*!
		\since YSS 0.18.0
		\return 虚拟文档的参数部分（紧凑 JSON）；其余scheme返回空字符串。
	*/
	QString YSSURI::getQuery() const {
		const YSSCore::Editor::VirtualFilePath* path = d->virtualPath();
		return path ? path->getParam() : QString();
	}

	/*!
		\since YSS 0.18.0
		\return 本机路径（原生分隔符）或 QRC 的 \c{:} 前缀形式；其余scheme返回空字符串。
	*/
	QString YSSURI::getLocalPath() const {
		if (const QFileInfo* info = d->local()) {
			return QDir::toNativeSeparators(info->absoluteFilePath());
		}
		if (d->SchemeType == Scheme::Resource) {
			return d->resourcePathText();
		}
		return QString();
	}

	/*!
		\since YSS 0.18.0
		\return \c doc scheme的 \c{@ext!name?param} 字符串；其余scheme返回空字符串。
	*/
	QString YSSURI::getVirtualPath() const {
		const YSSCore::Editor::VirtualFilePath* path = d->virtualPath();
		return path ? path->toString() : QString();
	}

	/*!
		\since YSS 0.18.0
		\return 文件名；虚拟文档返回其名称部分。

		本机路径直接使用 \l{QFileInfo::fileName()}，因此以分隔符结尾的路径没有文件名。
	*/
	QString YSSURI::getFileName() const {
		if (const QFileInfo* info = d->local()) {
			return info->fileName();
		}
		if (const Visindigo::Network::SshPath* path = d->sshPath()) {
			return path->getFileName();
		}
		if (const YSSCore::Editor::VirtualFilePath* path = d->virtualPath()) {
			return path->getFileName();
		}
		return QFileInfo(d->resourcePathText()).fileName();
	}

	/*!
		\since YSS 0.18.0
		\return 去掉扩展名后的文件名，满足 \c{getFileMainName() + "." + getFileExt() == getFileName()}
		（无扩展名时等于 \l{getFileName()}）。

		本机路径用 \l{QFileInfo::completeBaseName()}，因此 \c{a.tar.gz} 的主名是 \c{a.tar}；
		远端与 QRC 资源按同样的规则切分自己的文件名；虚拟文档没有扩展名，返回整个名称。
	*/
	QString YSSURI::getFileMainName() const {
		if (const QFileInfo* info = d->local()) {
			return info->completeBaseName();
		}
		if (d->SchemeType == Scheme::VirtualFile) {
			return this->getFileName();
		}
		return QFileInfo(this->getFileName()).completeBaseName();
	}

	/*!
		\since YSS 0.18.0
		\return 扩展名，不含点，保持原始大小写；没有扩展名时返回空字符串。

		虚拟文档没有扩展名概念，返回空字符串；它的 FileServer ID 用 \l{getHost()} 取。
	*/
	QString YSSURI::getFileExt() const {
		if (const QFileInfo* info = d->local()) {
			return info->suffix();
		}
		if (d->SchemeType == Scheme::VirtualFile) {
			return QString();
		}
		return QFileInfo(this->getFileName()).suffix();
	}

	/*!
		\since YSS 0.18.0
		\return 上一级目录的URI；已在根上、或是虚拟文档/QRC 资源时返回无效URI。
	*/
	YSSURI YSSURI::parent() const {
		if (const QFileInfo* info = d->local()) {
			if (info->isRoot()) {
				return YSSURI();
			}
			return fromLocalPath(info->absolutePath());
		}
		if (const Visindigo::Network::SshPath* path = d->sshPath()) {
			const Visindigo::Network::SshPath parentPath = path->parent();
			if (not parentPath.isValid()) {
				return YSSURI();
			}
			YSSURIPrivate* data = new YSSURIPrivate();
			data->SchemeType = Scheme::SSH;
			data->Value = parentPath;
			return YSSURI(data);
		}
		return YSSURI();
	}

	/*!
		\since YSS 0.18.0
		\a root 作为根的URI。
		\return 本URI是否与 \a root 同scheme、同authority，且路径在 \a root 之下（含相等）。

		只有本机路径与远端路径有目录层级；虚拟文档既没有 authority 也没有目录，
		与 QRC 资源一样总是返回 false。
	*/
	bool YSSURI::isUnder(const YSSURI& root) const {
		if (d->SchemeType != root.d->SchemeType) {
			return false;
		}
		if (const QFileInfo* info = d->local()) {
			const QFileInfo* rootInfo = root.d->local();
			if (rootInfo == nullptr) {
				return false;
			}
			const Qt::CaseSensitivity cs = localCaseSensitivity();
			const QString path = info->absoluteFilePath();
			const QString rootPath = rootInfo->absoluteFilePath();
			if (path.compare(rootPath, cs) == 0) {
				return true;
			}
			QString prefix = rootPath;
			if (not prefix.endsWith(QChar('/'))) {
				prefix += QChar('/');
			}
			return path.startsWith(prefix, cs);
		}
		const Visindigo::Network::SshPath* path = d->sshPath();
		const Visindigo::Network::SshPath* rootPath = root.d->sshPath();
		if (path == nullptr or rootPath == nullptr) {
			return false;
		}
		return path->isUnder(*rootPath);
	}

	/*!
		\since YSS 0.18.0
		\a root 作为根的URI。
		\return 相对 \a root 的路径；本URI与 \a root 相等、或不在其下时返回空字符串。
	*/
	QString YSSURI::relativePathTo(const YSSURI& root) const {
		if (d->SchemeType != root.d->SchemeType) {
			return QString();
		}
		if (const QFileInfo* info = d->local()) {
			const QFileInfo* rootInfo = root.d->local();
			if (rootInfo == nullptr) {
				return QString();
			}
			const Qt::CaseSensitivity cs = localCaseSensitivity();
			const QString path = info->absoluteFilePath();
			const QString rootPath = rootInfo->absoluteFilePath();
			if (path.compare(rootPath, cs) == 0) {
				return QString();
			}
			QString prefix = rootPath;
			if (not prefix.endsWith(QChar('/'))) {
				prefix += QChar('/');
			}
			if (not path.startsWith(prefix, cs)) {
				return QString();
			}
			return path.mid(prefix.size());
		}
		const Visindigo::Network::SshPath* path = d->sshPath();
		const Visindigo::Network::SshPath* rootPath = root.d->sshPath();
		if (path == nullptr or rootPath == nullptr) {
			return QString();
		}
		return path->relativePathTo(*rootPath);
	}

	/*!
		\since YSS 0.18.0
		\a relativePath 相对于本URI的路径；若传入完整URI或虚拟文档路径，则直接返回解析结果。
		\return 解析后的URI；越出根、或本位虚拟文档/QRC 资源时返回无效URI。

		本机路径的拼接与规范化交给 \l{QFileInfo::absoluteFilePath()}；远端路径交给
		\l{Visindigo::Network::SshPath::resolve()}。
	*/
	YSSURI YSSURI::resolve(const QString& relativePath) const {
		if (d->SchemeType == Scheme::Unknown or d->SchemeType == Scheme::VirtualFile
			or d->SchemeType == Scheme::Resource) {
			return YSSURI();
		}
		const QString text = relativePath.trimmed();
		if (text.isEmpty()) {
			return *this;
		}
		if (YSSCore::Editor::VirtualFilePath::isVirtualFilePath(text)
			or Visindigo::Network::SshPath::isSshPath(text) or isResourceText(text)) {
			return fromString(text);
		}
		if (const QFileInfo* info = d->local()) {
			if (QFileInfo(text).isAbsolute()) {
				return fromLocalPath(text);
			}
			QString joined = info->absoluteFilePath();
			if (not joined.endsWith(QChar('/'))) {
				joined += QChar('/');
			}
			return fromLocalPath(QFileInfo(joined + text).absoluteFilePath());
		}
		const Visindigo::Network::SshPath* path = d->sshPath();
		if (path == nullptr) {
			return YSSURI();
		}
		const Visindigo::Network::SshPath resolved = path->resolve(text);
		if (not resolved.isValid()) {
			return YSSURI();
		}
		YSSURIPrivate* data = new YSSURIPrivate();
		data->SchemeType = Scheme::SSH;
		data->Value = resolved;
		return YSSURI(data);
	}

	/*!
		\since YSS 0.18.0
		\return 对应类型自己的规范字符串，可被 \l{fromString()} 原样解析回来。

		本机路径就是 \l{QFileInfo::absoluteFilePath()}，远端是
		\l{Visindigo::Network::SshPath::toString()}，虚拟文档是
		\l{YSSCore::Editor::VirtualFilePath::toString()}，QRC 是 \c{:/x}
		（\l{QFile} 与 \l{QFileInfo} 接受的资源路径形式）。
	*/
	QString YSSURI::toString() const {
		if (const QFileInfo* info = d->local()) {
			return info->absoluteFilePath();
		}
		if (const Visindigo::Network::SshPath* path = d->sshPath()) {
			return path->toString();
		}
		if (const YSSCore::Editor::VirtualFilePath* path = d->virtualPath()) {
			return path->toString();
		}
		if (d->SchemeType == Scheme::Resource) {
			return d->resourcePathText();
		}
		return QString();
	}

	/*!
		\since YSS 0.18.0
		\a seed 哈希种子。
		\return 与 qHash() 一致的哈希值。

		取值来自 \l{toString()}。Windows 上的本机路径不区分大小写，因此这里对哈希输入
		也做同样的折叠，保证 \c{a == b} 蕴含 \c{hash(a) == hash(b)}。
	*/
	std::size_t YSSURI::hash(std::size_t seed) const {
		const QString text = this->toString();
		if (d->SchemeType == Scheme::Local and not localPathsAreCaseSensitive()) {
			return ::qHash(text.toLower(), seed);
		}
		return ::qHash(text, seed);
	}

	/*!
		\since YSS 0.18.0
		\a other 另一个URI。
		\return 两者是否指向同一资源。

		先比较 scheme，再交给该 scheme 自己的相等语义：
		\list
		\li \l{Scheme::Local} 用 \l{QFileInfo::absoluteFilePath()}，Windows 上不区分大小写。
		\li \l{Scheme::SSH} 用 \l{Visindigo::Network::SshPath::operator==()}。
		\li \l{Scheme::VirtualFile} 逐项比较扩展名、名称与参数。
		\li \l{Scheme::Resource} 比较 \c{:} 前缀的资源路径，区分大小写。
		\endlist

		scheme 不同一律不相等；两个无效URI相等。
	*/
	bool YSSURI::operator==(const YSSURI& other) const {
		if (d->SchemeType != other.d->SchemeType) {
			return false;
		}
		switch (d->SchemeType) {
		case Scheme::Local: {
			const QFileInfo* left = d->local();
			const QFileInfo* right = other.d->local();
			if (left == nullptr or right == nullptr) {
				return left == right;
			}
			return left->absoluteFilePath().compare(right->absoluteFilePath(), localCaseSensitivity()) == 0;
		}
		case Scheme::SSH: {
			const Visindigo::Network::SshPath* left = d->sshPath();
			const Visindigo::Network::SshPath* right = other.d->sshPath();
			if (left == nullptr or right == nullptr) {
				return left == right;
			}
			return *left == *right;
		}
		case Scheme::VirtualFile: {
			const YSSCore::Editor::VirtualFilePath* left = d->virtualPath();
			const YSSCore::Editor::VirtualFilePath* right = other.d->virtualPath();
			if (left == nullptr or right == nullptr) {
				return left == right;
			}
			return left->getExt() == right->getExt()
				and left->getFileName() == right->getFileName()
				and left->getParam() == right->getParam();
		}
		case Scheme::Resource:
			return d->resourcePathText() == other.d->resourcePathText();
		default:
			return true;
		}
	}

	/*!
		\since YSS 0.18.0
		\a other 另一个URI。
		\return 两者是否指向不同资源。
	*/
	bool YSSURI::operator!=(const YSSURI& other) const {
		return not (*this == other);
	}

	/*!
		\since YSS 0.18.0
		\a other 另一个URI。
		\return 便于放进 QMap 的字典序比较结果，按 \l{toString()} 比较。

		\note Windows 上本机路径按 \l{absoluteFilePath()} 的原样比较，未折叠大小写；
		需要与 \l{operator==()} 完全一致的容器请使用 \l{hash()} 配对的哈希容器。
	*/
	bool YSSURI::operator<(const YSSURI& other) const {
		return this->toString() < other.toString();
	}
}
