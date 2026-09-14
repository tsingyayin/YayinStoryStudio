#ifndef Visindigo_Agent_MCP_h
#define Visindigo_Agent_MCP_h
#include <QtCore/qmap.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include "VIAgentCompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class MCPPrivate;
}
// Main
namespace Visindigo::Agent {
	class VIAgentAPI MCP {
		Q_GADGET;
	public:
		enum class Transport {
			Stdio,
			Sse,
			StreamableHttp
		};
		Q_ENUM(Transport)
	public:
		MCP();
		MCP(const QString& name);
		VICopyable(MCP);
		VIMoveable(MCP);
		~MCP();
	public:
		MCP& setName(const QString& name);
		MCP& setTransport(Transport transport);
		MCP& setUrl(const QUrl& url);
		MCP& setCommand(const QString& command);
		MCP& setArguments(const QStringList& arguments);
		MCP& setEnvironment(const QMap<QString, QString>& environment);
		MCP& setHeaders(const QMap<QString, QString>& headers);
		MCP& setEnabled(bool enabled);
		MCP& setAutoStart(bool autoStart);
	public:
		QString getName() const;
		Transport getTransport() const;
		QUrl getUrl() const;
		QString getCommand() const;
		QStringList getArguments() const;
		QMap<QString, QString> getEnvironment() const;
		QMap<QString, QString> getHeaders() const;
		bool isEnabled() const;
		bool isAutoStart() const;
		bool isValid() const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		MCPPrivate* d;
	};
}
#endif // Visindigo_Agent_MCP_h
