#ifndef Visindigo_Agent_Provider_h
#define Visindigo_Agent_Provider_h
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Agent/Model.h"
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class ProviderPrivate;
}
// Main
namespace Visindigo::Agent {
	class VisindigoAPI Provider {
	public:
		Provider();
		Provider(const QString& name);
		VICopyable(Provider);
		VIMoveable(Provider);
		~Provider();
	public:
		Provider& setName(const QString& name);
		Provider& addModel(const QString& modelId, qint32 priority = 0);
		Provider& removeModel(const QString& modelId);
		Provider& clearModels();
	public:
		QString getId() const;
		QString getName() const;
		QStringList getModelIds() const;
		qint32 getModelPriority(const QString& modelId) const;
		QString pickFirst(Model::Capabilities required) const;
		QString pickNext(const QString& afterModelId, Model::Capabilities required) const;
		bool isValid() const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		ProviderPrivate* d;
	};
}
#endif // Visindigo_Agent_Provider_h
