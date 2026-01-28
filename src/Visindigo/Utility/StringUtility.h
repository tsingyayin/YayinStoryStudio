#pragma once
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include "../Macro.h"
namespace Visindigo::Utility {
	class VisindigoAPI StringUtility {
	public:
		QStringList escapeSplit(const QString& raw, const QString& sep, QChar escapeChar = '\\', Qt::SplitBehaviorFlags behavior = Qt::KeepEmptyParts) const;
	};
}