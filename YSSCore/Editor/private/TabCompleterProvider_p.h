#pragma once
#include <QtCore/qobject.h>

namespace YSSCore::Editor {
	class TabCompleterItem;
	class TabCompleterProvider;
}
namespace YSSCore::__Private__ {
	class TabCompleterProviderPrivate {
		friend class YSSCore::Editor::TabCompleterProvider;
		friend class YSSCore::Editor::TabCompleterItem;

	};
}