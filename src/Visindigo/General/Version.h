#pragma once
#include <QtCore/qstring.h>
#include "../VICompileMacro.h"
#include "General/private/AUTO_VERSION.h"
// Visindigo version macro
#define Visindigo_VERSION_MAJOR 0
#define Visindigo_VERSION_MINOR 13
#define Visindigo_VERSION_PATCH 2

#define Visindigo_ABI_VERSION_MAJOR 13
#define Visindigo_ABI_VERSION_MINOR 0
#define Compiled_VIABI_Version Visindigo::General::Version(Visindigo_ABI_VERSION_MAJOR, Visindigo_ABI_VERSION_MINOR, 0)
#define Compiled_VIAPI_Version Visindigo::General::Version(Visindigo_VERSION_MAJOR, Visindigo_VERSION_MINOR, Visindigo_VERSION_PATCH, true, Visindigo_VERSION_BUILD, QString(Visindigo_VERSION_NICKNAME))
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
		static Version getAPIVersion();
		static Version getABIVersion();
		static bool isCompatibleAPIVersion(const Version& env, const Version& tar);
		static bool isCompatibleABIVersion(const Version& env, const Version& tar);
	public:
		Version();
		Version(quint32 major, quint32 minor, quint32 patch, bool useBuild = false, quint32 build = 0, const QString& nickName = "");
		Version(const QString& version);
		VIMoveable(Version);
		VICopyable(Version);
		~Version();
		bool operator>(const Version& other) const;
		bool operator<(const Version& other) const;
		bool operator==(const Version& other) const;
		bool operator!=(const Version& other) const;
		bool operator>=(const Version& other) const;
		bool operator<=(const Version& other) const;
		QString toString() const;
		void setVersion(const QString& version);
		void setVersion(quint32 major, quint32 minor, quint32 patch, bool useBuild, quint32 build = 0, const QString& nickName = "");
		void setNickName(const QString& name);
		quint32 getMajor() const;
		quint32 getMinor() const;
		quint32 getPatch() const;
		bool getUseBuild() const;
		quint32 getBuild() const;
		QString getNickName() const;
		
	private:
		VersionPrivate* d;
	};
}