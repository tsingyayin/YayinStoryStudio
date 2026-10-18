#ifndef Visindigo_Network_SshPath_h
#define Visindigo_Network_SshPath_h
#include "VICompileMacro.h"
#include "VIMacro.h"
#include <QtCore/qhashfunctions.h>
#include <QtCore/qstring.h>
#include <cstddef>
// Forward declarations
namespace Visindigo::Network {
	class SshPathPrivate;
}
// Main
namespace Visindigo::Network {
	class VisindigoAPI SshPath {
	public:
		static bool isSshPath(const QString& text);
		static SshPath fromString(const QString& text);
		static SshPath fromParts(const QString& user, const QString& host, qint32 port, const QString& path);
	public:
		SshPath();
		VIMoveable(SshPath);
		VICopyable(SshPath);
		~SshPath();
	public:
		bool isValid() const;
		QString getUser() const;
		QString getHost() const;
		qint32 getPort() const;
		qint32 getEffectivePort() const;
		QString getAuthority() const;
		QString getPath() const;
		QString getFileName() const;
	public:
		SshPath parent() const;
		bool isUnder(const SshPath& root) const;
		QString relativePathTo(const SshPath& root) const;
		SshPath resolve(const QString& relativePath) const;
	public:
		QString toString() const;
		QString toKey() const;
		std::size_t hash() const;
		bool operator==(const SshPath& other) const;
		bool operator!=(const SshPath& other) const;
		bool operator<(const SshPath& other) const;
	private:
		explicit SshPath(SshPathPrivate* data);
		SshPathPrivate* d;
	};

	inline std::size_t qHash(const SshPath& path, std::size_t seed = 0) noexcept {
		return ::qHash(path.toKey(), seed);
	}
}
#endif // Visindigo_Network_SshPath_h
