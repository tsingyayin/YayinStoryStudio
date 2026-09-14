#ifndef Visindigo_Agent_Prompt_h
#define Visindigo_Agent_Prompt_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "VIAgentCompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class PromptPrivate;
	class Message;
}
// Main
namespace Visindigo::Agent {
	class VIAgentAPI Prompt {
	public:
		Prompt();
		Prompt(const QString& name, const QString& content);
		VICopyable(Prompt);
		VIMoveable(Prompt);
		~Prompt();
	public:
		Prompt& setName(const QString& name);
		Prompt& setDescription(const QString& description);
		Prompt& setContent(const QString& content);
		Prompt& setTags(const QStringList& tags);
	public:
		QString getName() const;
		QString getDescription() const;
		QString getContent() const;
		QStringList getTags() const;
		QStringList getVariables() const;
		bool isValid() const;
	public:
		Message render(const QMap<QString, QString>& variables) const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		PromptPrivate* d;
	};
}
#endif // Visindigo_Agent_Prompt_h
