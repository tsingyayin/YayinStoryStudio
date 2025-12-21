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
		//vgDebugF << names;
		QString currentName = names.first();
		QObject* child = object->findChild<QObject*>(currentName);
		if (!child) {
			//vgDebugF << "No such child" << currentName;
			return nullptr;
		}
		if (names.size() > 1) {
			return findChildObject(child, names.mid(1));
		}
		else {
			return child;
		}
	}

	/*!
		\class Visindigo::Utility::QtSSHelper
		\inmodule Visindigo
		\brief 此类提供了一些Qt信号槽相关的辅助函数。
		\since Visindigo 0.13.0

		此类提供一些用于辅助Qt信号槽机制的静态函数。它们在处理嵌套对象路径和深度连接信号槽时特别有用。

		特别需要注意的是，本类内所有深度访问均支持用"::"分隔的名称路径，以便访问嵌套对象。
	*/

	/*!
		\a obj 是要搜索其子对象的父对象。
		\a namePath 是用"::"分隔的子对象名称路径。
		\since Visindigo 0.13.0

		此函数根据给定的名称路径在对象层次结构中查找子对象。找不到时返回 nullptr。
	*/
	QObject* QtSSHelper::findChildObject(QObject* obj, const QString& namePath) {
		if (!obj) {
			vgErrorF << "obj is null";
			return nullptr;
		}
		QStringList parts = namePath.split("::");
		return Visindigo::Utility::findChildObject(obj, parts);
	}

	/*!
		\a emiter 是信号发出者对象所属对象树的根对象。
		\a signalName 是要连接的信号名称，支持用"::"分隔的嵌套路径。
		\a reciver 是槽接收者对象所属对象树的根对象。
		\a slotName 是要连接的槽名称，支持用"::"分隔的嵌套路径。
		\a type 是连接类型，默认为自动连接。
		\since Visindigo 0.13.0

		此函数在嵌套对象中查找指定的信号和槽，并将它们连接起来。它支持通过"::"分隔的名称路径来访问嵌套对象。

		不存在嵌套查找时，等同于使用SIGNAL和SLOT宏语法的QObject::connect调用。

		因此该函数与双字符写法的QObject::connect具有相同缺陷，即信号和槽的参数类型无法被编译器检查。如果
		不匹配，则只能在运行时发现错误。

		注：在Visindigo中，四种connect写法一般只出现其中三种，即：
		\list
		\li 1. 双字符写法的QObject::connect（emiter, SIGNAL(...), reciver, SLOT(...)）
		\li 2. 双函数指针写法的QObject::connect (emiter, &EmitterType::signal, reciver, &ReciverType::slot）
		\li 3. Lambda写法的QObject::connect (emiter, &EmitterType::signal, [...](...) { ... }）
		\endlist

		还有一种写法是前函数指针，后Lambda（我们称为函子写法），即(emiter, &EmitterType::signal, context, [...](...) { ... }），
		Visindigo规范上拒绝该写法，因为不利于代码可读性。
		与此同时，也只将Lambda写法限定为只操作本类本实例内部的对象，以避免代码可读性问题。
	*/
	void QtSSHelper::deepConnect(QObject* emiter, const QString& signalName, QObject* reciver, const QString& slotName, Qt::ConnectionType type) {
		if (!emiter) {
			vgErrorF << "emiter is null";
			return;
		}
		if (!reciver) {
			vgErrorF << "reciver is null";
			return;
		}
		QString finalSignalName = signalName;
		if (signalName.contains("::")) {
			QStringList parts = signalName.split("::");
			finalSignalName = parts.last();
			parts.removeLast();
			if (!parts.isEmpty()) { // empty means current object
				emiter = Visindigo::Utility::findChildObject(emiter, parts);
			}
		}
		QString finalSlotName = slotName;
		if (slotName.contains("::")) {
			QStringList parts = slotName.split("::");
			finalSlotName = parts.last();
			parts.removeLast();
			if (!parts.isEmpty()) {
				reciver = Visindigo::Utility::findChildObject(reciver, parts);
			}
		}
		//vgDebugF << finalSignalName << finalSlotName;
		if (!emiter){
			vgErrorF << "emiter not found for" << signalName;
			return;
		}
		if (!reciver){
			vgErrorF << "reciver not found for" << slotName;
			return;
		}
		QObject::connect(emiter, VI_VAR_SIGNAL(finalSignalName), reciver, VI_VAR_SLOT(finalSlotName), type);
	}
}