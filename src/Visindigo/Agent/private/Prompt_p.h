#ifndef Visindigo_Agent_private_Prompt_p_h
#define Visindigo_Agent_private_Prompt_p_h
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Agent/Prompt.h"
namespace Visindigo::Agent {
	class PromptPrivate {
		friend class Prompt;
	public:
		QString Name;
		QString Description;
		QString Content;
		QStringList Tags;
	};
}
#endif // Visindigo_Agent_private_Prompt_p_h
