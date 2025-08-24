#pragma once
#include <QtCore/qstring.h>
namespace Visindigo::Widgets {
	class MultiButton;
	class MultiButtonGroup;
}
namespace Visindigo::__Private__ {
	class MultiButtonGroupPrivate;
}
namespace Visindigo::__Private__ {
	class MultiButtonPrivate {
		friend class Visindigo::Widgets::MultiButton;
		friend class Visindigo::Widgets::MultiButtonGroup;
		friend class MultiButtonGroupPrivate;
	protected:
		QString PressedStyleSheet;
		QString HoveredStyleSheet;
		QString NormalStyleSheet;
		bool Pressed = false;
		bool Hovered = false;
	};
}