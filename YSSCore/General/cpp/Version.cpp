#include "../Version.h"

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
	};
	Version::Version(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build)
	{
		p = new VersionPrivate;
		setVersion(major, minor, patch, useBuild, build);
	}
	Version::Version(const QString& version)
	{
		p = new VersionPrivate;
		setVersion(version);
	}
	Version::Version(const Version& other)
	{
		p = new VersionPrivate;
		p->major = other.p->major;
		p->minor = other.p->minor;
		p->patch = other.p->patch;
		p->useBuild = other.p->useBuild;
		p->build = other.p->build;
	}
	Version::Version(Version&& other) noexcept
	{
		p = other.p;
		other.p = nullptr;
	}
	Version& Version::operator=(const Version& other)
	{
		if (this != &other) {
			p->major = other.p->major;
			p->minor = other.p->minor;
			p->patch = other.p->patch;
			p->useBuild = other.p->useBuild;
			p->build = other.p->build;
		}
		return *this;
	}
	Version& Version::operator=(Version&& other) noexcept
	{
		if (this != &other) {
			if (p)
				delete p;
			p = other.p;
			other.p = nullptr;
		}
		return *this;
	}
	Version::~Version()
	{
		if (p)
			delete p;
	}
	bool Version::operator>(const Version& other) const
	{
		if (p->major > other.p->major) {
			return true;
		}
		else if (p->major == other.p->major) {
			if (p->minor > other.p->minor) {
				return true;
			}
			else if (p->minor == other.p->minor) {
				if (p->patch > other.p->patch) {
					return true;
				}
				else if (p->patch == other.p->patch) {
					if (p->useBuild && other.p->useBuild) {
						if (p->build > other.p->build) {
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
		if (p->major < other.p->major) {
			return true;
		}
		else if (p->major == other.p->major) {
			if (p->minor < other.p->minor) {
				return true;
			}
			else if (p->minor == other.p->minor) {
				if (p->patch < other.p->patch) {
					return true;
				}
				else if (p->patch == other.p->patch) {
					if (p->useBuild && other.p->useBuild) {
						if (p->build < other.p->build) {
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
		if (p->major == other.p->major && p->minor == other.p->minor && p->patch == other.p->patch)
		{
			if (p->useBuild && other.p->useBuild)
			{
				return p->build == other.p->build;
			}
			else if (!p->useBuild && !other.p->useBuild)
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
		QString version = QString("%1.%2.%3").arg(p->major).arg(p->minor).arg(p->patch);
		if (p->useBuild)
			version += QString(".%1").arg(p->build);
		return version;
	}
	void Version::setVersion(const QString& version)
	{
		QStringList list = version.split(".");
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
		setVersion(major, minor, patch, useBuild, build);
	}
	void Version::setVersion(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build)
	{
		p->major = major;
		p->minor = minor;
		p->patch = patch;
		p->useBuild = useBuild;
		p->build = build;
	}
	quint32 Version::getMajor() const
	{
		return p->major;
	}
	quint32 Version::getMinor() const
	{
		return p->minor;
	}
	quint32 Version::getPatch() const
	{
		return p->patch;
	}
	bool Version::getUseBuild() const
	{
		return p->useBuild;
	}
	quint32 Version::getBuild() const
	{
		return p->build;
	}

}