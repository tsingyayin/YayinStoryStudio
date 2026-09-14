#ifndef Visindigo_Agent_private_Provider_p_h
#define Visindigo_Agent_private_Provider_p_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include "Agent/Provider.h"
namespace Visindigo::Agent {
	class ProviderPrivate {
		friend class Provider;
	public:
		QString Id;
		QString Name;
		QMap<QString, qint32> ModelPriorities;
	};
}
#endif // Visindigo_Agent_private_Provider_p_h
