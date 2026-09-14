#ifndef Visindigo_Agent_private_Center_p_h
#define Visindigo_Agent_private_Center_p_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include "Agent/MCP.h"
#include "Agent/Model.h"
#include "Agent/Prompt.h"
#include "Agent/Provider.h"
#include "Agent/Skill.h"
namespace Visindigo::Agent {
	class Center;
	class Dialog;
	class Function;

	class CenterPrivate {
		friend class Center;
		friend class DialogPrivate;
	public:
		static Center* Instance;
		QMap<QString, Model> Models;
		QMap<QString, Provider> Providers;
		QMap<QString, Skill> Skills;
		QMap<QString, MCP> MCPs;
		QMap<QString, Prompt> Prompts;
		QMap<QString, Function*> Functions;
		QMap<QString, Dialog*> Dialogs;
		QString DefaultProviderId;
	};
}
#endif // Visindigo_Agent_private_Center_p_h
