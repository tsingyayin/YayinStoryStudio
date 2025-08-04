#pragma once
#include <QtCore/qstring.h>
#include "../Macro.h"
namespace YSSCore::Utility {
	class YSSCoreAPI StringUtility {
	public:
		const QString& getWordContent(const QString& lineContent, qint32 position, qint32* startposition = nullptr) const;
	};
}