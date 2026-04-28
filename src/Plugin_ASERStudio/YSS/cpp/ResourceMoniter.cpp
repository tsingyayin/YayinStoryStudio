#include "YSS/ResourceMoniter.h"
#include <Utility/FileUtility.h>
#include <QtCore/qmap.h>
#include <QtCore/qfilesystemwatcher.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <General/Log.h>
namespace ASERStudio::YSS {
	class ResourceMoniterPrivate {
		friend class ResourceMoniter;
	protected:
		QStringList Backgrounds;
		QMap<QString, QStringList> CharaDiffs;
		QStringList Musics;
		QStringList SoundEffects;
		QString ProjectPath;
		QFileSystemWatcher* Watcher = nullptr;
		static ResourceMoniter* Instance;
	};

	ResourceMoniter* ResourceMoniterPrivate::Instance = nullptr;

	ResourceMoniter* ResourceMoniter::getInstance() {
		if (!ResourceMoniterPrivate::Instance) {
			ResourceMoniterPrivate::Instance = new ResourceMoniter();
		}
		return ResourceMoniterPrivate::Instance;
	}

	ResourceMoniter::ResourceMoniter() {
		d = new ResourceMoniterPrivate();
		d->Watcher = new QFileSystemWatcher(this);
		connect(d->Watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path) {
			refresh();
			});
	}

	ResourceMoniter::~ResourceMoniter() {
		delete d;
	}

	QStringList ResourceMoniter::getBackground() {
		return d->Backgrounds;
	}

	QStringList ResourceMoniter::getCharacters() {
		return d->CharaDiffs.keys();
	}

	QStringList ResourceMoniter::getCharaDiff(const QString& chara) {
		return d->CharaDiffs.value(chara);
	}

	QStringList ResourceMoniter::getMusic() {
		return d->Musics;
	}

	QStringList ResourceMoniter::getSoundEffect() {
		return d->SoundEffects;
	}

	void ResourceMoniter::changeProjectPath(const QString& path) {
		d->Watcher->removePaths(d->Watcher->directories());
		d->Watcher->addPath(path + "/Resources");
		d->ProjectPath = path;
		refresh();
	}

	void ResourceMoniter::refresh(ResourceTypes types) {
		if (types.testAnyFlag(ResourceType::Background)) {
			auto absFiles = Visindigo::Utility::FileUtility::fileFilter(d->ProjectPath + "/Resources/Background", { "*.png" });
			QStringList backgrounds;
			for (const QString& file : absFiles) {
				QFileInfo info(file);
				backgrounds.append(info.baseName());
			}
			d->Backgrounds = backgrounds;
		}
		if (types.testAnyFlag(ResourceType::Character)) {
			QMap<QString, QStringList> charaDiffs;
			for (const QString& chara : Visindigo::Utility::FileUtility::folderFilter(d->ProjectPath + "/Resources/Char_Picture")) {
				QDir charaAbsDir(chara);
				QString charaName = charaAbsDir.dirName();
				QStringList absDiffList = Visindigo::Utility::FileUtility::fileFilter(d->ProjectPath + "/Resources/Char_Picture/" + charaName, { "*.png" });
				QStringList diffList;
				for (const QString& file : absDiffList) {
					QFileInfo info(file);
					diffList.append(info.baseName());
				}
				vgDebug << diffList;
				charaDiffs.insert(charaName, diffList);
			}
			d->CharaDiffs = charaDiffs;
		}
		if (types.testAnyFlag(ResourceType::Music)) {
			auto absMusics = Visindigo::Utility::FileUtility::fileFilter(d->ProjectPath + "/Resources/Music", { "*.wav" });
			QStringList musics;
			for (const QString& file : absMusics) {
				QFileInfo info(file);
				musics.append(info.baseName());
			}

			d->Musics = musics;
		}
		if (types.testAnyFlag(ResourceType::SoundEffect)) {
			auto soundEffects = Visindigo::Utility::FileUtility::fileFilter(d->ProjectPath + "Resources/SoundEffect", { "*.wav" });
			QStringList soundEffectList;
			for (const QString& file : soundEffects) {
				QFileInfo info(file);
				soundEffectList.append(info.baseName());
			}
			d->SoundEffects = soundEffectList;
		}
	}
}