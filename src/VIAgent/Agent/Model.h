#ifndef Visindigo_Agent_Model_h
#define Visindigo_Agent_Model_h
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include "VIAgentCompileMacro.h"
// Forward declarations
class QJsonValue;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class ModelPrivate;
}
// Main
namespace Visindigo::Agent {
	class VIAgentAPI Model {
		Q_GADGET;
	public:
		enum class Capability {
			None = 0,
			Text = 1 << 0,
			Image = 1 << 1,
			Audio = 1 << 2,
			ToolUse = 1 << 3
		};
		Q_DECLARE_FLAGS(Capabilities, Capability)
		Q_FLAG(Capabilities)
	public:
		Model();
		VICopyable(Model);
		VIMoveable(Model);
		~Model();
	public:
		Model& setName(const QString& name);
		Model& setUrl(const QUrl& url);
		Model& setToken(const QString& token);
		Model& setTimeoutMs(qint32 ms);
		Model& setIdleTimeoutMs(qint32 ms);
		Model& setCapabilities(Capabilities capabilities);
		Model& addCapability(Capability capability);
		Model& setParameter(const QString& key, const QJsonValue& value);
		Model& setTemperature(qreal value);
		Model& setTopP(qreal value);
		Model& setMaxTokens(qint32 value);
	public:
		QString getId() const;
		QString getName() const;
		QUrl getUrl() const;
		QString getToken() const;
		qint32 getTimeoutMs() const;
		qint32 getIdleTimeoutMs() const;
		Capabilities getCapabilities() const;
		bool hasCapability(Capability capability) const;
		Visindigo::Utility::JsonConfig getParameters() const;
		bool isValid() const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		ModelPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(Model::Capabilities)
}
#endif // Visindigo_Agent_Model_h
