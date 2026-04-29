#include "YSS/ResourceMoniter.h"
#include <Utility/FileUtility.h>
#include <QtCore/qmap.h>
#include <QtCore/qfilesystemwatcher.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <General/Log.h>
#include <Utility/JsonConfig.h>
namespace ASERStudio::YSS {
	class ResourceMoniterPrivate {
		friend class ResourceMoniter;
	protected:
		QStringList Backgrounds;
		QMap<QString, QStringList> CharaDiffs;
		QStringList Musics;
		QStringList SoundEffects;
		QString ProjectPath;
		QStringList OfficialBackgrounds;
		QMap<QString, QStringList> OfficialCharaDiffs;
		QStringList OfficialMusics;
		QStringList OfficialSoundEffects;
		QFileSystemWatcher* Watcher = nullptr;
		QFileSystemWatcher* OfficialWatcher = nullptr;
		static ResourceMoniter* Instance;

		void refreshOfficial() {
			auto collections = Visindigo::Utility::JsonConfig();
			collections.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_collections.json"));
			auto aliases = Visindigo::Utility::JsonConfig();
			aliases.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_aliases.json"));
			auto character = Visindigo::Utility::JsonConfig();
			character.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json"));
			vgDebug << ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json";
			QStringList rawCharaKeysTachies = collections.keys("characterTachies");
			QMap<QString, QStringList> officialCharaDiffs;
			for (auto key : rawCharaKeysTachies) {
				QString alias = aliases.getString("characterTachies." + key);
				alias = alias.isEmpty() ? key : alias;
				auto variationNames = character.getObject("characterTachieOfficialConfigs." + key + ".variationNames");
				QStringList variations;
				for (auto variationKey : variationNames.keys()) {
					variations.append(variationNames.getString(variationKey));
				}
				officialCharaDiffs.insert(alias, variations);
			}
			QStringList rawCharaKeysSpine = collections.keys("characterDynamics");
			for (auto key : rawCharaKeysSpine) {
				QString alias = aliases.getString("characterDynamics." + key);
				alias = alias.isEmpty() ? key : alias;
				auto variationNames = character.getObject("characterSpineOfficialConfigs." + key + ".variationNames");
				QStringList variations;
				for (auto variationKey : variationNames.keys()) {
					variations.append(variationNames.getString(variationKey));
				}
				officialCharaDiffs.insert(alias, variations);
			}
			OfficialCharaDiffs = officialCharaDiffs;

			QStringList backgroundRawKeys = collections.keys("backgrounds");
			QStringList officialBackgrounds;
			for (auto key : backgroundRawKeys) {
				QString alias = aliases.getString("backgrounds." + key);
				if (alias.isEmpty()) {
					officialBackgrounds.append(key);
				}
				else {
					officialBackgrounds.append(alias);
				}
			}
			OfficialBackgrounds = officialBackgrounds;

			QStringList musicRawKeys = collections.keys("musics");
			QStringList officialMusics;
			for (auto key : musicRawKeys) {
				QString alias = aliases.getString("musics." + key);
				if (alias.isEmpty()) {
					officialMusics.append(key);
				}
				else {
					officialMusics.append(alias);
				}
			}
			OfficialMusics = officialMusics;

			QStringList soundEffectRawKeys = collections.keys("soundEffects");
			QStringList officialSoundEffects;
			for (auto key : soundEffectRawKeys) {
				QString alias = aliases.getString("soundEffects." + key);
				if (alias.isEmpty()) {
					officialSoundEffects.append(key);
				}
				else {
					officialSoundEffects.append(alias);
				}
			}
			OfficialSoundEffects = officialSoundEffects;
		}
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
		d->OfficialWatcher = new QFileSystemWatcher(this);
		d->OfficialWatcher->addPath(getASERStandardConfigPath() + "/Configs");
		d->refreshOfficial();
		connect(d->OfficialWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path) {
			d->refreshOfficial();
			});
	}

	ResourceMoniter::~ResourceMoniter() {
		delete d;
	}

	QString ResourceMoniter::getASERStandardConfigPath() {
		return QDir::homePath()+"/AppData/LocalLow/Gradus/ASE-Remake";
	}

	QStringList ResourceMoniter::getBackground() {
		return d->Backgrounds + d->OfficialBackgrounds;
	}

	QStringList ResourceMoniter::getCharacters() {
		return d->CharaDiffs.keys() + d->OfficialCharaDiffs.keys();
	}

	QStringList ResourceMoniter::getCharaDiff(const QString& chara) {
		return d->CharaDiffs.value(chara) + d->OfficialCharaDiffs.value(chara);
	}

	QStringList ResourceMoniter::getMusic() {
		return d->Musics + d->OfficialMusics;
	}

	QStringList ResourceMoniter::getSoundEffect() {
		return d->SoundEffects + d->OfficialSoundEffects;
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