#include "../QtSSHelper.h"
#include <QtCore/qobject.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstringlist.h>
#include "../../General/Log.h"
#ifndef QT_NO_DEBUG
#define VI_VAR_SIGNAL(name) qFlagLocation(QString(QString::number(QSIGNAL_CODE)+name+QString(QLOCATION)).toUtf8().constData())
#define VI_VAR_SLOT(name) qFlagLocation(QString(QString::number(QSLOT_CODE)+name+QString(QLOCATION)).toUtf8().constData())
#else
#define VI_VAR_SIGNAL(name) QString(QString::number(QSIGNAL_CODE)+name).toUtf8().constData()
#define VI_VAR_SLOT(name) QString(QString::number(QSLOT_CODE)+name).toUtf8().constData()
#endif
namespace Visindigo::Utility {
	static QObject* findChildObject(QObject* object, const QStringList& names) {
		//yDebugF << names;
		QString currentName = names.first();
		QObject* child = object->findChild<QObject*>(currentName);
		if (!child) {
			//yDebugF << "No such child" << currentName;
			return nullptr;
		}
		if (names.size() > 1) {
			return findChildObject(child, names.mid(1));
		}
		else {
			return child;
		}
	}
	void QtSSHelper::deepConnect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName, Qt::ConnectionType type) {
		if (!emiter || !reciver) {
			yErrorF << "emiter or reciver is null";
		}
		QString finalSignalName = signalName;
		if (signalName.contains("::")) {
			QStringList parts = signalName.split("::");
			finalSignalName = parts.last();
			parts.removeLast();
			emiter = findChildObject(emiter, parts);
		}
		QString finalSlotName = slotName;
		if (slotName.contains("::")) {
			QStringList parts = slotName.split("::");
			finalSlotName = parts.last();
			parts.removeLast();
			reciver = findChildObject(reciver, parts);
		}
		//yDebugF << finalSignalName << finalSlotName;
		if (!emiter || !reciver) {
			yErrorF << "emiter or reciver is null after findChildObject";
		}
		QObject::connect(emiter, VI_VAR_SIGNAL(finalSignalName), reciver, VI_VAR_SLOT(finalSlotName), type);
	}
}