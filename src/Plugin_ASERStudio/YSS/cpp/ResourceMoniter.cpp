#include "YSS/ResourceMoniter.h"
#include <Utility/FileUtility.h>
#include <QtCore/qmap.h>
#include <QtCore/qfilesystemwatcher.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <General/Log.h>
#include <Utility/JsonConfig.h>
#include <QtCore/qset.h>
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
		QStringList totalBackgrounds;
		QMap<QString, QStringList> totalCharaDiffs;
		QStringList totalMusics;
		QStringList totalSoundEffects;
		QFileSystemWatcher* Watcher = nullptr;
		QFileSystemWatcher* OfficialWatcher = nullptr;
		static ResourceMoniter* Instance;

		void refreshOfficial() {
			auto collections = Visindigo::Utility::JsonConfig();
			collections.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_collections.json"));

			auto aliases = Visindigo::Utility::JsonConfig();
			if (Visindigo::Utility::FileUtility::isFileExist(ProjectPath + "/Configs/officialAssets_aliases.json")) {
				QString content = Visindigo::Utility::FileUtility::readAll(ProjectPath + "/Configs/officialAssets_aliases.json");
				auto rtn = aliases.parse(content);
				if (rtn.error != QJsonParseError::NoError) {
					aliases.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_aliases.json"));
				}
			}
			else {
				aliases.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_aliases.json"));
			}

			auto character = Visindigo::Utility::JsonConfig();
			if (Visindigo::Utility::FileUtility::isFileExist(ProjectPath + "/Configs/officialAssets_userConfigs.json")) {
				QString content = Visindigo::Utility::FileUtility::readAll(ProjectPath + "/Configs/officialAssets_userConfigs.json");
				auto rtn = character.parse(content);
				if (rtn.error != QJsonParseError::NoError) {
					character.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json"));
				}
			}
			else {
				character.parse(Visindigo::Utility::FileUtility::readAll(ResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json"));
			}
			
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
			QStringList officialBackgroundAliases;
			for (auto key : backgroundRawKeys) {
				QString alias = aliases.getString("backgrounds." + key);
				QString collection = collections.getString("backgrounds." + key);
				if (alias == collection) {
					officialBackgrounds.append(key);
				}
				else {
					if (not alias.isEmpty()) {
						officialBackgroundAliases.append(alias);
					}
					else {
						officialBackgrounds.append(key);
					}
				}
			}
			OfficialBackgrounds = officialBackgroundAliases + officialBackgrounds;

			QStringList musicRawKeys = collections.keys("musics");
			QStringList officialMusics;
			QStringList officialMusicAliases;
			for (auto key : musicRawKeys) {
				QString alias = aliases.getString("musics." + key);
				QString collection = collections.getString("musics." + key);
				if (alias == collection) {
					officialMusics.append(key);
				}
				else {
					if (not alias.isEmpty()) {
						officialMusicAliases.append(alias);
					}
					else {
						officialMusics.append(key);
					}
				}
			}
			OfficialMusics = officialMusicAliases + officialMusics;

			QStringList soundEffectRawKeys = collections.keys("soundEffects");
			QStringList officialSoundEffects;
			QStringList officialSoundEffectAliases;
			for (auto key : soundEffectRawKeys) {
				QString alias = aliases.getString("soundEffects." + key);
				QString collection = collections.getString("soundEffects." + key);
				if (alias == collection) {
					officialSoundEffects.append(key);
				}
				else {
					if (not alias.isEmpty()) {
						officialSoundEffectAliases.append(alias);
					}
					else {
						officialSoundEffects.append(key);
					}
				}
			}
			OfficialSoundEffects = officialSoundEffectAliases + officialSoundEffects;
		}

		void refreshTotalBackgrounds() {
			totalBackgrounds = Backgrounds;
			QSet<QString> backgroundSet;
			for (const QString& background : Backgrounds) {
				backgroundSet.insert(background);
			}
			for (const QString& background : OfficialBackgrounds) {
				if (not backgroundSet.contains(background)) {
					totalBackgrounds.append(background);
				}
			}
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
		return QDir::homePath() + "/AppData/LocalLow/Gradus/ASE-Remake";
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
				//vgDebug << diffList;
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
			auto soundEffects = Visindigo::Utility::FileUtility::fileFilter(d->ProjectPath + "/Resources/SoundEffect", { "*.wav" });
			QStringList soundEffectList;
			for (const QString& file : soundEffects) {
				QFileInfo info(file);
				soundEffectList.append(info.baseName());
			}
			d->SoundEffects = soundEffectList;
		}
	}
}