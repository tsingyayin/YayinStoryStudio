#ifndef Visindigo_Agent_private_Skill_p_h
#define Visindigo_Agent_private_Skill_p_h
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Agent/Skill.h"
namespace Visindigo::Agent {
	class SkillPrivate {
		friend class Skill;
	public:
		QString Name;
		QString Description;
		QString Instruction;
		bool Enabled = true;
		QStringList Tags;
		QStringList FunctionIds;
	};
}
#endif // Visindigo_Agent_private_Skill_p_h
