#include "../ShortCutHelper.h"
#include "../private/qnamespace_key.h"
#include <QtCore/qmetaobject.h>

namespace Visindigo::Utility {
	Qt::Key ShortCutHelper::toKey(const QString& name) {
		QMetaEnum metaEnum = QMetaEnum::fromType<Visindigo::__Private__::QtNamespaceKey::KeyEnum>();
		int value = metaEnum.keyToValue(name.toUtf8().constData());
		return static_cast<Qt::Key>(value);
	}
}