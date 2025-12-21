#pragma once
#include "../Macro.h"
#include <QObject>
#include <Qt>
class QString;

namespace Visindigo::Utility {
	class VisindigoAPI QtSSHelper {
	public:
		static QObject* findChildObject(QObject* obj, const QString& namePath);
		static void deepConnect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName, Qt::ConnectionType type = Qt::ConnectionType::AutoConnection);
	};
}