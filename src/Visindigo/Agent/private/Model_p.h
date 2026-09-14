#ifndef Visindigo_Agent_private_Model_p_h
#define Visindigo_Agent_private_Model_p_h
#include <QtCore/qjsonvalue.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include "Agent/Model.h"
namespace Visindigo::Agent {
	class ModelPrivate {
		friend class Model;
	public:
		QString Id;
		QString Name;
		QUrl Url;
		QString Token;
		qint32 TimeoutMs = 60000;
		qint32 IdleTimeoutMs = 60000;
		Model::Capabilities Capabilities = Model::Capability::Text;
		QMap<QString, QJsonValue> Parameters;
	};
}
#endif // Visindigo_Agent_private_Model_p_h
