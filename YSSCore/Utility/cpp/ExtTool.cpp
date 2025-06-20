#include "../ExtTool.h"
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
namespace YSSCore::Utility {
	/*! 
		\class YSSCore::Utility::ExtTool
		\brief 此类为Yayin Story Studio 提供文件扩展名的相关操作
		\since Yayin Story Studio 0.13.0
		\inmodule YSSCore
	*/

	/*! 
		\a ext 文件扩展名
		\a description 文件描述
		\since Yayin Story Studio 0.13.0
		注册文件扩展名。此函数仅在Windows平台下有效。
	*/
	void ExtTool::registerFileExtension(const QString& ext, const QString& description, const QString& iconPath) {
		QString PROGRAMNAME = "YayinStoryStudio";
		QString CLASSNAME = PROGRAMNAME + "." + ext;
		QString REGPATH = "HKEY_CURRENT_USER\\Software\\Classes";
		QSettings REG(REGPATH, QSettings::NativeFormat);
		REG.setValue("/" + CLASSNAME+ "/.", description);
		REG.setValue("/" + CLASSNAME+ "/DefaultIcon/.", iconPath);
		REG.setValue("/." + ext + "/OpenWithProgIds/" + CLASSNAME, 0);
		REG.setValue("/." + ext + "/." , CLASSNAME);
		REG.sync();
	}
}