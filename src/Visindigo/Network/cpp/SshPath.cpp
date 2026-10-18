#include "Network/SshPath.h"
#include <QtCore/qstringlist.h>

namespace {
	constexpr qint32 DefaultSshPort = 22;

	const QString& sshSchemePrefix() {
		static const QString prefix = QStringLiteral("ssh://");
		return prefix;
	}

	bool containsSpace(const QString& text) {
		for (const QChar ch : text) {
			if (ch.isSpace()) {
				return true;
			}
		}
		return false;
	}

	// 主机名统一小写；IPv6 字面量必须带方括号，避免和无括号的 host:port 混淆。
	bool normalizeHost(const QString& raw, QString* out) {
		const QString host = raw.trimmed();
		if (host.isEmpty() || containsSpace(host)) {
			return false;
		}
		if (host.contains(QChar('?')) || host.contains(QChar('#'))) {
			return false;
		}
		if (host.startsWith(QChar('['))) {
			if (not host.endsWith(QChar(']')) or host.size() <= 2) {
				return false;
			}
			const QString inner = host.mid(1, host.size() - 2);
			if (inner.isEmpty() or inner.contains(QChar('[')) or inner.contains(QChar(']'))) {
				return false;
			}
			*out = host.toLower();
			return true;
		}
		if (host.contains(QChar(':'))) {
			return false;
		}
		*out = host.toLower();
		return true;
	}

	bool normalizeUser(const QString& raw, QString* out) {
		const QString user = raw.trimmed();
		if (containsSpace(user)) {
			return false;
		}
		*out = user;
		return true;
	}

	// 只做 POSIX 词法规范化：反斜杠是普通字符，不能像 Windows 路径那样转换。
	// 返回 false 表示 .. 越过了根。
	bool normalizePosixPath(const QString& input, bool absolute, QString* out) {
		QStringList segments;
		const QStringList raw = input.split(QChar('/'), Qt::KeepEmptyParts);
		for (const QString& part : raw) {
			if (part.isEmpty() or part == QStringLiteral(".")) {
				continue;
			}
			if (part == QStringLiteral("..")) {
				if (segments.isEmpty()) {
					return false;
				}
				segments.removeLast();
				continue;
			}
			segments.append(part);
		}
		QString joined = segments.join(QChar('/'));
		if (absolute and not joined.startsWith(QChar('/'))) {
			joined.prepend(QChar('/'));
		}
		*out = joined;
		return true;
	}

	bool parseAuthority(const QString& authority, QString* user, QString* host, qint32* port) {
		QString hostPart = authority;
		QString userPart;
		const qsizetype at = authority.lastIndexOf(QChar('@'));
		if (at >= 0) {
			userPart = authority.left(at);
			hostPart = authority.mid(at + 1);
		}

		QString normalizedUser;
		if (not normalizeUser(userPart, &normalizedUser)) {
			return false;
		}

		QString normalizedHost;
		qint32 parsedPort = -1;
		if (hostPart.startsWith(QChar('['))) {
			const qsizetype close = hostPart.indexOf(QChar(']'));
			if (close < 0) {
				return false;
			}
			const QString hostText = hostPart.left(close + 1);
			const QString afterHost = hostPart.mid(close + 1);
			if (not afterHost.isEmpty()) {
				if (not afterHost.startsWith(QChar(':'))) {
					return false;
				}
				bool ok = false;
				const qint32 value = afterHost.mid(1).toInt(&ok);
				if (not ok or value <= 0 or value > 65535) {
					return false;
				}
				parsedPort = value;
			}
			if (not normalizeHost(hostText, &normalizedHost)) {
				return false;
			}
		}
		else {
			const qsizetype colon = hostPart.lastIndexOf(QChar(':'));
			if (colon >= 0) {
				bool ok = false;
				const qint32 value = hostPart.mid(colon + 1).toInt(&ok);
				if (not ok or value <= 0 or value > 65535) {
					return false;
				}
				parsedPort = value;
				hostPart = hostPart.left(colon);
			}
			if (not normalizeHost(hostPart, &normalizedHost)) {
				return false;
			}
		}

		*user = normalizedUser;
		*host = normalizedHost;
		*port = parsedPort;
		return true;
	}
}

namespace Visindigo::Network {
	class SshPathPrivate {
		friend class SshPath;
	protected:
		QString User;
		QString Host;
		qint32 Port = -1;
		QString Path;
	protected:
		qint32 effectivePort() const {
			return Port > 0 ? Port : DefaultSshPort;
		}
		bool isSameAuthorityAs(const SshPathPrivate& other) const {
			return User == other.User and Host == other.Host and effectivePort() == other.effectivePort();
		}
		bool parse(const QString& text);
	};

	bool SshPathPrivate::parse(const QString& text) {
		const QString trimmed = text.trimmed();
		if (not trimmed.startsWith(sshSchemePrefix(), Qt::CaseInsensitive)) {
			return false;
		}
		const QString rest = trimmed.mid(sshSchemePrefix().size());
		const qsizetype slash = rest.indexOf(QChar('/'));
		const QString authority = slash < 0 ? rest : rest.left(slash);
		const QString pathText = slash < 0 ? QStringLiteral("/") : rest.mid(slash);
		if (authority.isEmpty()) {
			return false;
		}
		if (not parseAuthority(authority, &User, &Host, &Port)) {
			return false;
		}
		QString normalized;
		if (not normalizePosixPath(pathText, true, &normalized)) {
			return false;
		}
		Path = normalized;
		return true;
	}

	/*!
		\class Visindigo::Network::SshPath
		\since Visindigo 0.18.0
		\inmodule Visindigo
		\brief SSH 远端路径：连接身份（authority）加绝对 POSIX 路径。

		本类只描述"哪个连接上的哪个路径"，不持有连接、凭据或会话。
		它用于给 YSSURI 提供 SSH 分支的身份，也用于连接索引与文件索引。

		\section1 形式

		\badcode
			ssh://[user@]host[:port]/absolute/path
		\endcode

		\list
		\li authority 为 \c{user@host:port}；host 在解析时转小写，端口缺省按 22 处理。
		\li 路径一律是远端 POSIX 绝对路径：以 \c{/} 开头，折叠重复分隔符与 \c{.}，
			解析 \c{..}；越过根的 \c{..} 判为无效。
		\li 反斜杠不作为分隔符，因为它在 POSIX 上是合法文件名字符。
		\li 不做百分号解码，也不展开 \c{~}。
		\endlist

		\section1 身份

		\c{toString()} / \c{toKey()} 总是输出规范形式，端口总是写出（缺省 22），因此
		\c{host} 与 \c{host:22} 是同一个身份。路径区分大小写。连接索引只用
		\c{getAuthority()}，文件身份用 \c{toKey()}。
	*/

	/*!
		\fn bool Visindigo::Network::SshPath::isSshPath(const QString& text)
		\since Visindigo 0.18.0

		判断文本是否以 \c{ssh://} 开头。本函数不做完整校验，是否合法请用
		\l{fromString()} 判断。

		\a text 待检测文本。

		\return 前缀匹配返回 true。
	*/
	bool SshPath::isSshPath(const QString& text) {
		return text.trimmed().startsWith(sshSchemePrefix(), Qt::CaseInsensitive);
	}

	/*!
		\fn Visindigo::Network::SshPath Visindigo::Network::SshPath::fromString(const QString& text)
		\since Visindigo 0.18.0

		从 \c{ssh://[user@]host[:port]/path} 解析。

		路径缺省为 \c{/}。未加方括号的 IPv6、非法端口、空主机、越过根的
		\c{..} 都会得到无效对象。

		\a text 文本形式。

		\return 解析结果；无法解析时返回无效对象。
	*/
	SshPath SshPath::fromString(const QString& text) {
		SshPathPrivate* data = new SshPathPrivate();
		if (not data->parse(text)) {
			delete data;
			return SshPath();
		}
		return SshPath(data);
	}

	/*!
		\fn Visindigo::Network::SshPath Visindigo::Network::SshPath::fromParts(const QString& user, const QString& host, qint32 port, const QString& path)
		\since Visindigo 0.18.0

		从各个部分构造。\a user 可为空；\a host 不能带端口，IPv6 必须带方括号；
		\a port 小于等于 0 表示使用默认端口。

		\a path 会按绝对路径规范化；相对形式会补上前导 \c{/}。

		\a user SSH 用户名。
		\a host 主机名或带方括号的 IPv6 字面量。
		\a port 端口，小于等于 0 表示默认值。
		\a path 远端路径。

		\return 构造结果；参数不合法时返回无效对象。
	*/
	SshPath SshPath::fromParts(const QString& user, const QString& host, qint32 port, const QString& path) {
		if (port > 65535) {
			return SshPath();
		}
		QString normalizedUser;
		QString normalizedHost;
		QString normalizedPath;
		if (not normalizeUser(user, &normalizedUser)) {
			return SshPath();
		}
		if (not normalizeHost(host, &normalizedHost)) {
			return SshPath();
		}
		if (not normalizePosixPath(path, true, &normalizedPath)) {
			return SshPath();
		}
		SshPathPrivate* data = new SshPathPrivate();
		data->User = normalizedUser;
		data->Host = normalizedHost;
		data->Port = port > 0 ? port : -1;
		data->Path = normalizedPath;
		return SshPath(data);
	}

	/*!
		\since Visindigo 0.18.0

		构造一个无效对象。
	*/
	SshPath::SshPath() {
		this->d = new SshPathPrivate();
	}

	VIMoveable_Impl(SshPath);
	VICopyable_Impl(SshPath);

	/*!
		\since Visindigo 0.18.0

		析构函数。
	*/
	SshPath::~SshPath() {
		delete this->d;
	}

	// 接管一份已经解析好的私有数据，工厂函数内部使用。
	SshPath::SshPath(SshPathPrivate* data) {
		this->d = data;
	}

	/*!
		\fn bool Visindigo::Network::SshPath::isValid() const
		\since Visindigo 0.18.0

		\return 主机与绝对路径都存在时返回 true。
	*/
	bool SshPath::isValid() const {
		return d != nullptr and not d->Host.isEmpty() and d->Path.startsWith(QChar('/'));
	}

	/*!
		\fn QString Visindigo::Network::SshPath::getUser() const
		\since Visindigo 0.18.0

		\return SSH 用户名；未指定时为空字符串。
	*/
	QString SshPath::getUser() const {
		return d == nullptr ? QString() : d->User;
	}

	/*!
		\fn QString Visindigo::Network::SshPath::getHost() const
		\since Visindigo 0.18.0

		\return 小写主机名；IPv6 字面量包含方括号。
	*/
	QString SshPath::getHost() const {
		return d == nullptr ? QString() : d->Host;
	}

	/*!
		\fn qint32 Visindigo::Network::SshPath::getPort() const
		\since Visindigo 0.18.0

		\return 显式给出的端口；未指定时返回 -1。
	*/
	qint32 SshPath::getPort() const {
		return d == nullptr ? -1 : d->Port;
	}

	/*!
		\fn qint32 Visindigo::Network::SshPath::getEffectivePort() const
		\since Visindigo 0.18.0

		\return 实际使用的端口；未指定时返回 22，无效对象返回 -1。
	*/
	qint32 SshPath::getEffectivePort() const {
		return this->isValid() ? d->effectivePort() : -1;
	}

	/*!
		\fn QString Visindigo::Network::SshPath::getAuthority() const
		\since Visindigo 0.18.0

		\return 连接身份 \c{user@host:port}；端口总是写出（缺省 22）。
	*/
	QString SshPath::getAuthority() const {
		if (not this->isValid()) {
			return QString();
		}
		QString text;
		if (not d->User.isEmpty()) {
			text += d->User;
			text += QChar('@');
		}
		text += d->Host;
		text += QChar(':');
		text += QString::number(d->effectivePort());
		return text;
	}

	/*!
		\fn QString Visindigo::Network::SshPath::getPath() const
		\since Visindigo 0.18.0

		\return 规范化的远端 POSIX 绝对路径。
	*/
	QString SshPath::getPath() const {
		return d == nullptr ? QString() : d->Path;
	}

	/*!
		\fn QString Visindigo::Network::SshPath::getFileName() const
		\since Visindigo 0.18.0

		\return 路径的最后一段；根路径返回空字符串。
	*/
	QString SshPath::getFileName() const {
		if (not this->isValid() or d->Path == QStringLiteral("/")) {
			return QString();
		}
		const qsizetype slash = d->Path.lastIndexOf(QChar('/'));
		return slash < 0 ? d->Path : d->Path.mid(slash + 1);
	}

	/*!
		\fn Visindigo::Network::SshPath Visindigo::Network::SshPath::parent() const
		\since Visindigo 0.18.0

		\return 上一级路径；根路径或无效对象返回无效对象。
	*/
	SshPath SshPath::parent() const {
		if (not this->isValid() or d->Path == QStringLiteral("/")) {
			return SshPath();
		}
		const qsizetype slash = d->Path.lastIndexOf(QChar('/'));
		if (slash < 0) {
			return SshPath();
		}
		SshPathPrivate* data = new SshPathPrivate(*d);
		data->Path = slash == 0 ? QStringLiteral("/") : d->Path.left(slash);
		return SshPath(data);
	}

	/*!
		\fn bool Visindigo::Network::SshPath::isUnder(const SshPath& root) const
		\since Visindigo 0.18.0

		判断是否与 \a root 属于同一个 authority，且路径在 \a root 之下（含相等）。

		\a root 作为根的路径。

		\return 在根之下返回 true。
	*/
	bool SshPath::isUnder(const SshPath& root) const {
		if (not this->isValid() or not root.isValid() or not d->isSameAuthorityAs(*root.d)) {
			return false;
		}
		if (d->Path == root.d->Path) {
			return true;
		}
		QString prefix = root.d->Path;
		if (not prefix.endsWith(QChar('/'))) {
			prefix += QChar('/');
		}
		return d->Path.startsWith(prefix);
	}

	/*!
		\fn QString Visindigo::Network::SshPath::relativePathTo(const SshPath& root) const
		\since Visindigo 0.18.0

		\a root 作为根的路径。

		\return 相对 \a root 的路径；相等、不在其下或 authority 不同时返回空字符串。
	*/
	QString SshPath::relativePathTo(const SshPath& root) const {
		if (not this->isValid() or not root.isValid() or not d->isSameAuthorityAs(*root.d)) {
			return QString();
		}
		if (d->Path == root.d->Path) {
			return QString();
		}
		QString prefix = root.d->Path;
		if (not prefix.endsWith(QChar('/'))) {
			prefix += QChar('/');
		}
		if (not d->Path.startsWith(prefix)) {
			return QString();
		}
		return d->Path.mid(prefix.size());
	}

	/*!
		\fn Visindigo::Network::SshPath Visindigo::Network::SshPath::resolve(const QString& relativePath) const
		\since Visindigo 0.18.0

		在同一个 authority 下解析路径。

		\a relativePath 相对路径；若传入 \c{ssh://} 文本或绝对路径，则直接按对应形式解析。
		\c{..} 越过根时返回无效对象。

		\return 解析结果。
	*/
	SshPath SshPath::resolve(const QString& relativePath) const {
		if (not this->isValid()) {
			return SshPath();
		}
		const QString text = relativePath.trimmed();
		if (text.isEmpty()) {
			return *this;
		}
		if (SshPath::isSshPath(text)) {
			return SshPath::fromString(text);
		}
		// 不把其它 scheme 的文本当成相对文件名。
		if (text.contains(QStringLiteral("://"))) {
			return SshPath();
		}
		QString joined;
		if (text.startsWith(QChar('/'))) {
			joined = text;
		}
		else {
			joined = d->Path;
			if (not joined.endsWith(QChar('/'))) {
				joined += QChar('/');
			}
			joined += text;
		}
		QString normalized;
		if (not normalizePosixPath(joined, true, &normalized)) {
			return SshPath();
		}
		SshPathPrivate* data = new SshPathPrivate(*d);
		data->Path = normalized;
		return SshPath(data);
	}

	/*!
		\fn QString Visindigo::Network::SshPath::toString() const
		\since Visindigo 0.18.0

		\return 规范形式 \c{ssh://user@host:port/path}；无效对象返回空字符串。
	*/
	QString SshPath::toString() const {
		if (not this->isValid()) {
			return QString();
		}
		return sshSchemePrefix() + this->getAuthority() + d->Path;
	}

	/*!
		\fn QString Visindigo::Network::SshPath::toKey() const
		\since Visindigo 0.18.0

		\return 用于比较与哈希的规范键；当前与 \l{toString()} 相同。
	*/
	QString SshPath::toKey() const {
		return this->toString();
	}

	/*!
		\fn std::size_t Visindigo::Network::SshPath::hash() const
		\since Visindigo 0.18.0

		\return 与 qHash() 一致的哈希值。
	*/
	std::size_t SshPath::hash() const {
		return ::qHash(this->toKey(), 0);
	}

	/*!
		\fn bool Visindigo::Network::SshPath::operator==(const SshPath& other) const
		\since Visindigo 0.18.0

		\a other 另一个路径。

		\return 两者是否表示同一个远端文件。
	*/
	bool SshPath::operator==(const SshPath& other) const {
		return this->toKey() == other.toKey();
	}

	/*!
		\fn bool Visindigo::Network::SshPath::operator!=(const SshPath& other) const
		\since Visindigo 0.18.0

		\a other 另一个路径。

		\return 两者是否表示不同文件。
	*/
	bool SshPath::operator!=(const SshPath& other) const {
		return not (*this == other);
	}

	/*!
		\fn bool Visindigo::Network::SshPath::operator<(const SshPath& other) const
		\since Visindigo 0.18.0

		\a other 另一个路径。

		\return 便于放进 QMap 的字典序比较结果。
	*/
	bool SshPath::operator<(const SshPath& other) const {
		return this->toKey() < other.toKey();
	}
}
