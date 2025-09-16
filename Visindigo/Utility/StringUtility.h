#pragma once
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include "../Macro.h"
namespace Visindigo::Utility {
	class VisindigoAPI StringUtility {
	public:
		const QString& getWordContent(const QString& lineContent, qint32 position, qint32* startposition = nullptr) const;
	};
}