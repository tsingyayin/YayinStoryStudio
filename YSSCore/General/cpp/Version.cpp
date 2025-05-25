#include "../Version.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qregularexpression.h>
#include "../private/AUTO_VERSION.h"

namespace YSSCore::General {
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
	};
	Version::Version(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build, const QString& nickName)
	{
		d = new VersionPrivate;
		setVersion(major, minor, patch, useBuild, build, nickName);
	}
	Version::Version(const QString& version)
	{
		d = new VersionPrivate;
		setVersion(version);
	}
	Version::Version(const Version& other)
	{
		d = new VersionPrivate;
		d->major = other.d->major;
		d->minor = other.d->minor;
		d->patch = other.d->patch;
		d->useBuild = other.d->useBuild;
		d->build = other.d->build;
	}
	Version::Version(Version&& other) noexcept
	{
		d = other.d;
		other.d = nullptr;
	}
	Version& Version::operator=(const Version& other)
	{
		if (this != &other) {
			d->major = other.d->major;
			d->minor = other.d->minor;
			d->patch = other.d->patch;
			d->useBuild = other.d->useBuild;
			d->build = other.d->build;
		}
		return *this;
	}
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
	Version::~Version()
	{
		if (d)
			delete d;
	}
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
	bool Version::operator!=(const Version& other) const
	{
		return !(*this == other);
	}
	bool Version::operator>=(const Version& other) const
	{
		return *this > other || *this == other;
	}
	bool Version::operator<=(const Version& other) const
	{
		return *this < other || *this == other;
	}
	QString Version::toString() const
	{
		QString version = QString("%1.%2.%3").arg(d->major).arg(d->minor).arg(d->patch);
		if (d->useBuild) {
			version += QString(".%1").arg(d->build);
		}
		if (!d->nickName.isEmpty()) {
			version += QString(" [%1]").arg(d->nickName);
		}
		return version;
	}
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
	void Version::setVersion(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build, const QString& nickName)
	{
		d->major = major;
		d->minor = minor;
		d->patch = patch;
		d->useBuild = useBuild;
		d->build = build;
		d->nickName = nickName;
	}
	void Version::setNickName(const QString& name)
	{
		d->nickName = name;
	}
	quint32 Version::getMajor() const
	{
		return d->major;
	}
	quint32 Version::getMinor() const
	{
		return d->minor;
	}
	quint32 Version::getPatch() const
	{
		return d->patch;
	}
	bool Version::getUseBuild() const
	{
		return d->useBuild;
	}
	quint32 Version::getBuild() const
	{
		return d->build;
	}
	QString Version::getNickName() const
	{
		return d->nickName;
	}
	Version Version::getYSSVersion() {
		return Version(YSSCore_VERSION_MAJOR,
			YSSCore_VERSION_MINOR,
			YSSCore_VERSION_PATCH,
			true,
			YSSCore_VERSION_BUILD,
			YSSCore_VERSION_NICKNAME);
	}
}