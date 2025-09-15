#pragma once
#include "../Macro.h"
class QObject;
class QString;

namespace Visindigo::Utility {
	class VisindigoAPI QtSSHelper {
	public:
		static void connect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName);
	};
}