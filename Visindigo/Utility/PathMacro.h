#pragma once
#include "../Macro.h"
#include <functional>
// Forward declarations
class QString;
namespace Visindigo::Utility {
	class PathMacroPrivate;
}
// Main
namespace Visindigo::Utility {
	class VisindigoAPI PathMacro {
		friend class PathMacroPrivate;
	public:
		static PathMacro* getInstance();
		PathMacro();
		~PathMacro();
		QString replace(QString raw);
		void addMacro(QString macro, std::function<QString(QString)> replacer);
	private:
		PathMacroPrivate* d;
	};
}
// Global Macros
#define YSSPathMacro Visindigo::Utility::PathMacro::getInstance()->replace