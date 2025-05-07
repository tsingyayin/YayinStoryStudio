#include "../ExtTool.h"
#include <QSettings>

namespace YSSCore::Utility {
	void ExtTool::registerFileExtension(const QString& ext, const QString& description) {
		QString CLASSNAME = "YayinStoryStudio";
		QString REGPATH = "HKEY_CURRENT_USER\\Software\\Classes";
		QSettings REG(REGPATH, QSettings::NativeFormat);
		REG.setValue("/" + CLASSNAME + "/.", description);
		REG.setValue("/." + ext + "/OpenWithProgIds/" + CLASSNAME, 0);
		REG.sync();
	}
}