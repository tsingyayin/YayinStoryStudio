#pragma once
#include "../Macro.h"
#include <QtCore/qobject.h>

class QString;

namespace Visindigo::Utility {
	class VisindigoAPI QtSSHelper {
	public:
		static void deepConnect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName, Qt::ConnectionType type = Qt::ConnectionType::AutoConnection);
	};
}