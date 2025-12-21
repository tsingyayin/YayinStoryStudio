#include "../Version.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qregularexpression.h>
#include "../private/AUTO_VERSION.h"

namespace Visindigo::General {
	class VersionPrivate
	{
		friend class Version;
	protected:
		quint32 major;
		quint32 minor;
		quint32 patch;
		bool useBuild;
		quint32 build;
		QString nickName;
		static Version APIVersion;
		static Version ABIVersion;
	};
	Version VersionPrivate::APIVersion = Version(Visindigo_VERSION_MAJOR, Visindigo_VERSION_MINOR, Visindigo_VERSION_PATCH, true, Visindigo_VERSION_BUILD, Visindigo_VERSION_NICKNAME);
	Version VersionPrivate::ABIVersion = Version(Visindigo_ABI_VERSION_MAJOR, Visindigo_ABI_VERSION_MINOR, 0);

	/*!
		\class Visindigo::General::Version
		\brief 在现有主流版本号表示方法中找到的一个折中方案。
		\inmodule Visindigo
		\inheaderfile General/Version.h
		\since Visindigo 0.13.0

		版本号类，表示软件的版本信息。版本号由主版本号、次版本号、修订号、可选的构建号和可选的昵称组成。

		这个类支持复制和移动，并且提供了比较运算符来比较不同版本号的大小关系。
	*/

	/*!
		默认构造函数，创建一个版本号为0.0.0的对象。默认不使用构建号，昵称为空。
	*/
	Version::Version()
	{
		d = new VersionPrivate;
		d->major = 0;
		d->minor = 0;
		d->patch = 0;
		d->useBuild = false;
		d->build = 0;
		d->nickName = "";
	}

	/*!
		\a major 主版本号
		\a minor 次版本号
		\a patch 修订号
		\a useBuild 是否使用构建号
		\a build 构建号
		\a nickName 昵称

		构建号默认不使用且值为0，昵称默认为空字符串。
	*/
	Version::Version(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build, const QString& nickName)
	{
		d = new VersionPrivate;
		setVersion(major, minor, patch, useBuild, build, nickName);
	}

	/*!
		\a version 版本号字符串，格式为 "major.minor.patch[.build] [nickName]"。

		如果字符串格式不正确，则版本号保持为默认值0.0.0。
	*/
	Version::Version(const QString& version)
	{
		d = new VersionPrivate;
		setVersion(version);
	}

	/*!
		\a other 另一个版本号对象。

		复制构造函数，创建一个与 \a other 相同的版本号对象。
	*/
	Version::Version(const Version& other)
	{
		d = new VersionPrivate;
		d->major = other.d->major;
		d->minor = other.d->minor;
		d->patch = other.d->patch;
		d->useBuild = other.d->useBuild;
		d->build = other.d->build;
		d->nickName = other.d->nickName;
	}

	/*!
		\a other 另一个版本号对象。

		移动构造函数，转移 \a other 的资源到新对象中，\a other 变为空对象。
	*/
	Version::Version(Version&& other) noexcept
	{
		d = other.d;
		other.d = nullptr;
	}

	/*!
		\a other 另一个版本号对象。

		复制赋值运算符，将 \a other 的值赋给当前对象。
	*/
	Version& Version::operator=(const Version& other)
	{
		if (this != &other) {
			d->major = other.d->major;
			d->minor = other.d->minor;
			d->patch = other.d->patch;
			d->useBuild = other.d->useBuild;
			d->build = other.d->build;
			d->nickName = other.d->nickName;
		}
		return *this;
	}

	/*!
		\a other 另一个版本号对象。

		移动赋值运算符，将 \a other 的资源转移到当前对象中，\a other 变为空对象。
	*/
	Version& Version::operator=(Version&& other) noexcept
	{
		if (this != &other) {
			if (d)
				delete d;
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}

	/*!
		析构函数，释放资源。
	*/
	Version::~Version()
	{
		if (d)
			delete d;
	}

	/*!
		\a other 另一个版本号对象。

		大于运算符，比较当前对象与 \a other 的版本号大小。它依次比较主版本号、次版本号、修订号和构建号（如果使用）。
	*/
	bool Version::operator>(const Version& other) const
	{
		if (d->major > other.d->major) {
			return true;
		}
		else if (d->major == other.d->major) {
			if (d->minor > other.d->minor) {
				return true;
			}
			else if (d->minor == other.d->minor) {
				if (d->patch > other.d->patch) {
					return true;
				}
				else if (d->patch == other.d->patch) {
					if (d->useBuild && other.d->useBuild) {
						if (d->build > other.d->build) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	/*!
		\a other 另一个版本号对象。

		小于运算符，比较当前对象与 \a other 的版本号大小。它依次比较主版本号、次版本号、修订号和构建号（如果使用）。
	*/
	bool Version::operator<(const Version& other) const
	{
		if (d->major < other.d->major) {
			return true;
		}
		else if (d->major == other.d->major) {
			if (d->minor < other.d->minor) {
				return true;
			}
			else if (d->minor == other.d->minor) {
				if (d->patch < other.d->patch) {
					return true;
				}
				else if (d->patch == other.d->patch) {
					if (d->useBuild && other.d->useBuild) {
						if (d->build < other.d->build) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	/*!
		\a other 另一个版本号对象。

		等于运算符，比较当前对象与 \a other 的版本号是否相等。它比较主版本号、次版本号、修订号和构建号（如果使用）。
	*/
	bool Version::operator==(const Version& other) const
	{
		if (d->major == other.d->major && d->minor == other.d->minor && d->patch == other.d->patch)
		{
			if (d->useBuild && other.d->useBuild)
			{
				return d->build == other.d->build;
			}
			else if (!d->useBuild && !other.d->useBuild)
			{
				return true;
			}
		}
		return false;
	}

	/*!
		\a other 另一个版本号对象。

		不等于运算符，是等于运算符的取反。
	*/
	bool Version::operator!=(const Version& other) const {
		return !(*this == other);
	}

	/*!
		\a other 另一个版本号对象。

		大于等于运算符，是大于运算符和等于运算符的取或。
	*/
	bool Version::operator>=(const Version& other) const {
		return *this > other || *this == other;
	}

	/*!
		\a other 另一个版本号对象。

		小于等于运算符，是小于运算符和等于运算符的取或。
	*/
	bool Version::operator<=(const Version& other) const {
		return *this < other || *this == other;
	}

	/*!
		将版本号转换为字符串表示，格式为 "major.minor.patch[.build] [nickName]"。
	*/
	QString Version::toString() {
		QString version = QString("%1.%2.%3").arg(d->major).arg(d->minor).arg(d->patch);
		if (d->useBuild) {
			version += QString(".%1").arg(d->build);
		}
		if (!d->nickName.isEmpty()) {
			version += QString(" [%1]").arg(d->nickName);
		}
		return version;
	}

	/*!
		\a version 版本号字符串，格式为 "major.minor.patch[.build] [nickName]"。

		如果字符串格式不正确，则版本号保持不变。
	*/
	void Version::setVersion(const QString& version)
	{
		QRegularExpression nickNameRE("[\\[\\{\\(].+[\\]\\}\\)]");
		QRegularExpressionMatch match = nickNameRE.match(version);
		QString nickName, versionStr;
		if (match.hasMatch())
		{
			nickName = match.captured(0);
			versionStr = version;
			versionStr.remove(nickName);
		}
		else
		{
			versionStr = version;
		}
		QStringList list = versionStr.split(".");
		if (list.size() < 3)
			return;
		bool ok = false;
		quint32 major = list[0].toUInt(&ok);
		if (!ok)
			return;
		quint32 minor = list[1].toUInt(&ok);
		if (!ok)
			return;
		quint32 patch = list[2].toUInt(&ok);
		if (!ok)
			return;
		bool useBuild = false;
		quint32 build = 0;
		if (list.size() > 3)
		{
			useBuild = true;
			build = list[3].toUInt(&ok);
			if (!ok)
				return;
		}
		setVersion(major, minor, patch, useBuild, build, nickName);
	}

	/*!
		\a major 主版本号
		\a minor 次版本号
		\a patch 修订号
		\a useBuild 是否使用构建号
		\a build 构建号
		\a nickName 昵称
	*/
	void Version::setVersion(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build, const QString& nickName)
	{
		d->major = major;
		d->minor = minor;
		d->patch = patch;
		d->useBuild = useBuild;
		d->build = build;
		d->nickName = nickName;
	}

	/*!
		\a name 昵称
	*/
	void Version::setNickName(const QString& name)
	{
		d->nickName = name;
	}

	/*!
		返回主版本号。
	*/
	quint32 Version::getMajor() const
	{
		return d->major;
	}

	/*!
		返回次版本号。
	*/
	quint32 Version::getMinor() const
	{
		return d->minor;
	}
	/*!
		返回修订号。
	*/
	quint32 Version::getPatch() const
	{
		return d->patch;
	}
	/*!
		返回是否使用构建号。
	*/
	bool Version::getUseBuild() const
	{
		return d->useBuild;
	}
	/*!
		返回构建号。
	*/
	quint32 Version::getBuild() const
	{
		return d->build;
	}
	/*!
		返回昵称。
	*/
	QString Version::getNickName() const
	{
		return d->nickName;
	}

	/*!
		返回Visindigo在编译时的API版本号。
	*/
	Version Version::getAPIVersion() {
		return VersionPrivate::APIVersion;
	}

	/*!
		返回Visindigo在编译时的ABI版本号。
	*/
	Version Version::getABIVersion() {
		return VersionPrivate::ABIVersion;
	}
}

/*!
	\macro Visindigo_VERSION_MAJOR
	\relates Visindigo::General::Version

	定义Visindigo的主版本号。
*/

/*!
	\macro Visindigo_VERSION_MINOR
	\relates Visindigo::General::Version

	定义Visindigo的次版本号。
*/

/*!
	\macro Visindigo_VERSION_PATCH
	\relates Visindigo::General::Version

	定义Visindigo的修订号。
*/

/*!
	\macro Visindigo_ABI_VERSION_MAJOR
	\relates Visindigo::General::Version

	定义Visindigo的ABI主版本号。
*/

/*!
	\macro Visindigo_ABI_VERSION_MINOR
	\relates Visindigo::General::Version

	定义Visindigo的ABI次版本号。
*/

/*!
	\macro Compiled_VIABI_Version
	\relates Visindigo::General::Version

	定义编译时的Visindigo ABI版本号。

	这和 Version::getABIVersion() 在使用上略有区别。

	因为它是被定义在头文件里的，因此在编译期就被确定，可以静态的留存在程序中，
	被用来保存编译时的ABI版本号信息。Visindigo::General::Plugin就使用此宏
	来固定编译ABI版本号。

	而 Version::getABIVersion() 则可在运行时调用，
	更多的是用来在运行时查询所使用的Visindigo库的ABI版本号信息。

	当编译版本与使用的库版本一致时，这两个版本号是相同的。
*/