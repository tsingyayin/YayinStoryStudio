#ifndef Visindigo_Agent_Skill_h
#define Visindigo_Agent_Skill_h
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class SkillPrivate;
}
// Main
namespace Visindigo::Agent {
	class VisindigoAPI Skill {
	public:
		Skill();
		Skill(const QString& name);
		VICopyable(Skill);
		VIMoveable(Skill);
		~Skill();
	public:
		Skill& setName(const QString& name);
		Skill& setDescription(const QString& description);
		Skill& setInstruction(const QString& instruction);
		Skill& setEnabled(bool enabled);
		Skill& setTags(const QStringList& tags);
	public:
		// 只记录函数 id；实际对象在执行时由 Center / Dialog 的函数表解析
		Skill& addFunction(const QString& functionId);
		Skill& removeFunction(const QString& id);
		bool hasFunction(const QString& id) const;
		QStringList getFunctionIds() const;
	public:
		QString getName() const;
		QString getDescription() const;
		QString getInstruction() const;
		bool isEnabled() const;
		QStringList getTags() const;
		bool isValid() const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		SkillPrivate* d;
	};
}
#endif // Visindigo_Agent_Skill_h
