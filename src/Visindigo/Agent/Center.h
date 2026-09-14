#ifndef Visindigo_Agent_Center_h
#define Visindigo_Agent_Center_h
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Agent/Dialog.h"
#include "Agent/Function.h"
#include "Agent/MCP.h"
#include "Agent/Model.h"
#include "Agent/Prompt.h"
#include "Agent/Provider.h"
#include "Agent/Skill.h"
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class CenterPrivate;
}
// Main
namespace Visindigo::Agent {
	class VisindigoAPI Center : public QObject {
		Q_OBJECT;
	public:
		static Center* getInstance();
	public:
		Center& addModel(const Model& model);
		Center& removeModel(const QString& id);
		Model getModel(const QString& id) const;
		QList<Model> getModels() const;
		QStringList getModelIds() const;
	public:
		Center& addProvider(const Provider& provider);
		Center& removeProvider(const QString& id);
		Provider getProvider(const QString& id) const;
		QList<Provider> getProviders() const;
	public:
		Center& addSkill(const Skill& skill);
		Center& removeSkill(const QString& name);
		Skill getSkill(const QString& name) const;
		QList<Skill> getSkills() const;
	public:
		Center& addMCP(const MCP& server);
		Center& removeMCP(const QString& name);
		MCP getMCP(const QString& name) const;
		QList<MCP> getMCPs() const;
	public:
		// 转移所有权；Center 负责释放
		Center& addFunction(Function* function);
		Center& removeFunction(const QString& id);
		Function* getFunction(const QString& id) const;
		QStringList getFunctionIds() const;
	public:
		Center& addPrompt(const Prompt& prompt);
		Center& removePrompt(const QString& name);
		Prompt getPrompt(const QString& name) const;
		QList<Prompt> getPrompts() const;
	public:
		Center& setDefaultProvider(const QString& id);
		QString getDefaultProvider() const;
	public:
		Dialog* createDialog();
		Dialog* restoreDialog(const Visindigo::Utility::JsonConfig& json);
		Visindigo::Utility::JsonConfig saveDialog(const Dialog* dialog) const;
		void removeDialog(const QString& id);
		Dialog* getDialog(const QString& id) const;
		QStringList getDialogIds() const;
	public:
		Dialog* cloneDialog(const QString& dialogId);
		Dialog* regenerate(const QString& dialogId);
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		Center();
		virtual ~Center();
		CenterPrivate* d;
	};
}
#endif // Visindigo_Agent_Center_h
