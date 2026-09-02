#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qmap.h>
#include "General/PluginBinaryHelper.h"
namespace Visindigo::General {
	class PluginBinaryHelperPrivate {
	protected:
		QString binaryFilePath;
		QMap<PluginBinaryHelper::Platform, QByteArray> platformBinaries;
		QStringList dependencies;
		QString pluginID;
		bool compressed;
	protected:
		bool loadBinary() {
		}
	};
}