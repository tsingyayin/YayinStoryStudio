#ifndef Visindigo_Agent_private_MCP_p_h
#define Visindigo_Agent_private_MCP_p_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include "Agent/MCP.h"
namespace Visindigo::Agent {
	class MCPPrivate {
		friend class MCP;
	public:
		QString Name;
		MCP::Transport TransportKind = MCP::Transport::Stdio;
		QUrl Url;
		QString Command;
		QStringList Arguments;
		QMap<QString, QString> Environment;
		QMap<QString, QString> Headers;
		bool Enabled = true;
		bool AutoStart = true;
	};
}
#endif // Visindigo_Agent_private_MCP_p_h
