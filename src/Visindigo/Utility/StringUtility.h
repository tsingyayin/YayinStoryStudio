#pragma once
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include "../VICompileMacro.h"
namespace Visindigo::Utility {
	class VisindigoAPI StringUtility {
	public:
		static qint32 getLevensteinDistance(const QString& s1, const QString& s2);
	};
}