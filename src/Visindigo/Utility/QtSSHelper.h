#ifndef Visindigo_Utility_QtSSHelper_h
#define Visindigo_Utility_QtSSHelper_h
#include "VICompileMacro.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qobject.h>
class QString;

namespace Visindigo::Utility {
	class VisindigoAPI QtSSHelper {
	public:
		static QObject* findChildObject(QObject* obj, const QString& namePath);
		static void deepConnect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName, Qt::ConnectionType type = Qt::ConnectionType::AutoConnection);
		static bool isDescendantsOf(QObject* child, QObject* parent);
	};
}
#endif // Visindigo_Utility_QtSSHelper_h