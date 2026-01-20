#include "Utility/ColorTool.h"

namespace Visindigo::Utility {
	bool ColorTool::isLightColor(const QColor& color) noexcept {
		return isLightColor(static_cast<quint8>(color.red()),
			static_cast<quint8>(color.green()),
			static_cast<quint8>(color.blue()));
	};
}