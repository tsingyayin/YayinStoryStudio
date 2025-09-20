#pragma once
#include <QtCore/qstring.h>
#include "../Macro.h"

// Visindigo version macro
#define Visindigo_VERSION_MAJOR 0
#define Visindigo_VERSION_MINOR 13
#define Visindigo_VERSION_PATCH 0

#define Visindigo_ABI_VERSION_MAJOR 12
#define Visindigo_ABI_VERSION_MINOR 0
#define Compiled_YSSABI_Version Visindigo::General::Version(Visindigo_ABI_VERSION_MAJOR, Visindigo_ABI_VERSION_MINOR, 0)

// Forward declarations
namespace Visindigo::General {
	class VersionPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI Version
	{
		friend class VersionPrivate;
	public:
		Version(quint32 major, quint32 minor, quint32 patch, bool useBuild = false, quint32 build = 0, const QString& nickName = "");
		Version(const QString& version);
		Version(const Version& other);
		Version(Version&& other) noexcept;
		Version& operator=(const Version& other);
		Version& operator=(Version&& other) noexcept;
		~Version();
		bool operator>(const Version& other) const;
		bool operator<(const Version& other) const;
		bool operator==(const Version& other) const;
		bool operator!=(const Version& other) const;
		bool operator>=(const Version& other) const;
		bool operator<=(const Version& other) const;
		QString toString();
		void setVersion(const QString& version);
		void setVersion(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build = 0, const QString& nickName = "");
		void setNickName(const QString& name);
		quint32 getMajor() const;
		quint32 getMinor() const;
		quint32 getPatch() const;
		bool getUseBuild() const;
		quint32 getBuild() const;
		QString getNickName() const;
		static Version getAPIVersion();
		static Version getABIVersion();
	private:
		VersionPrivate* d;
	};
}