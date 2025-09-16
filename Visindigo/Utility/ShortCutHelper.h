#pragma once
#include <QtCore/qnamespace.h>

namespace Visindigo::Utility {
	class ShortCutHelper {
		static Qt::Key toKey(const QString& keyName);
	};
}