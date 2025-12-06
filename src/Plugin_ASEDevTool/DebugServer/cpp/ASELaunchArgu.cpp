#include "../ASELaunchArgu.h"
#include <QtCore/qstring.h>
#include <Utility/JsonConfig.h>

class ASELaunchArguPrivate {
	public:
	QString workingFolder;
	QString mainFileName;
	ASELaunchArgu::SizeMode sizeMode;
};

ASELaunchArgu::ASELaunchArgu()
	: d(new ASELaunchArguPrivate()) {
	d->workingFolder = "";
	d->mainFileName = "";
	d->sizeMode = SizeMode::W1920H1080;
}

ASELaunchArgu::ASELaunchArgu(const QString& workingFolder, const QString& mainFileName, SizeMode mode)
	: d(new ASELaunchArguPrivate()) {
	d->workingFolder = workingFolder;
	d->mainFileName = mainFileName;
	d->sizeMode = mode;
}

VIMoveable_Impl(ASELaunchArgu)

VICopyable_Impl(ASELaunchArgu)

void ASELaunchArgu::setWorkingFolder(const QString& folder) {
	d->workingFolder = folder;
}

void ASELaunchArgu::setMainFileName(const QString& fileName) {
	d->mainFileName = fileName;
}

void ASELaunchArgu::setSizeMode(SizeMode mode) {
	d->sizeMode = mode;
}

QString ASELaunchArgu::getWorkingFolder() const {
	return d->workingFolder;
}

QString ASELaunchArgu::getMainFileName() const {
	return d->mainFileName;
}

ASELaunchArgu::SizeMode ASELaunchArgu::getSizeMode() const {
	return d->sizeMode;
}

QString ASELaunchArgu::toString() {
	Visindigo::Utility::JsonConfig config;
	config.setValue("path", d->workingFolder);
	config.setValue("fileName", d->mainFileName);
	config.setValue("sizeMode", static_cast<qint32>(d->sizeMode));
	return config.toString(QJsonDocument::Compact);
}