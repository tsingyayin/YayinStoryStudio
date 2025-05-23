#include "../PathMacro.h"
#include <QtCore/qmap.h>
#include <QtCore/qdir.h>
namespace YSSCore::Utility {
	class PathMacroPrivate {
		friend class PathMacro;
	protected:
		QMap<QString, std::function<QString(QString)>> Macros;
		static PathMacro* Instance;
	};
	PathMacro* PathMacroPrivate::Instance = nullptr;
	
	PathMacro* PathMacro::getInstance() {
		return PathMacroPrivate::Instance;
	}
	PathMacro::PathMacro() {
		d = new PathMacroPrivate;
		PathMacroPrivate::Instance = this;
		addMacro("$(ProgramPath)", [](QString macro)->QString {
			return QDir::currentPath();
		});
	}
	PathMacro::~PathMacro() {
		delete d;
	}

	QString PathMacro::replace(QString raw) {
		for (QString macro : d->Macros.keys()) {
			raw = raw.replace(macro, d->Macros[macro](raw));
		}
		return raw;
	}
	void PathMacro::addMacro(QString macro, std::function<QString(QString)> replacer) {
		if (!macro.startsWith("$(")) {
			macro = "$(" + macro;
		}
		if (!macro.endsWith(")")) {
			macro = macro + ")";
		}
		d->Macros[macro] = replacer;
	}
}