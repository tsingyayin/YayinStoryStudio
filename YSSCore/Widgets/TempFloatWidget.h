#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
#include <QtGui/qicon.h>

class QWidget;

namespace YSSCore::Widgets {
	class TempFloatWidget {
	public:
		QString spawnLineEdit(const QString& description, const QString& defaultValue, const QString& Title = "", const QIcon& icon = QIcon());
	};
}