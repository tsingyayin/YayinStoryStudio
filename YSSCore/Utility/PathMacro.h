#pragma once
#include "../Macro.h"
#include <functional>
class QString;

namespace YSSCore::Utility {
	class PathMacroPrivate;
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

#define YSSPathMacro YSSCore::Utility::PathMacro::getInstance()->replace