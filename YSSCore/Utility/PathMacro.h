#pragma once
#include "../Macro.h"
#include <functional>
// Forward declarations
class QString;
namespace YSSCore::Utility {
	class PathMacroPrivate;
}
// Main
namespace YSSCore::Utility {
	class YSSCoreAPI PathMacro {
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
#define YSSPathMacro YSSCore::Utility::PathMacro::getInstance()->replace