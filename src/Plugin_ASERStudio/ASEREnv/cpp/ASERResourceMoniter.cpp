#include "ASEREnv/ASERResourceMoniter.h"
#include <Utility/FileUtility.h>
#include <QtCore/qmap.h>
#include <QtCore/qfilesystemwatcher.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <General/Log.h>
#include <Utility/JsonConfig.h>
#include <QtCore/qset.h>
namespace ASERStudio::ASEREnv {
	class ASERResourceMoniterPrivate {
		friend class ASERResourceMoniter;
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
		static ASERResourceMoniter* Instance;

		void refreshOfficial() {
			auto collections = Visindigo::Utility::JsonConfig();
			collections.parse(Visindigo::Utility::FileUtility::readAll(ASERResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_collections.json"));
			auto aliases = Visindigo::Utility::JsonConfig();
			aliases.parse(Visindigo::Utility::FileUtility::readAll(ASERResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_aliases.json"));
			auto character = Visindigo::Utility::JsonConfig();
			character.parse(Visindigo::Utility::FileUtility::readAll(ASERResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json"));
			vgDebug << ASERResourceMoniter::getASERStandardConfigPath() + "/Configs/officialAssets_userConfigs.json";
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
	};

	ASERResourceMoniter* ASERResourceMoniterPrivate::Instance = nullptr;

	/*!
		\class ASERStudio::ASEREnv::ASERResourceMoniter
		\since ASERStudio 2.4.0
		\brief ASERResourceMonitor提供对ASER官方资源和项目资源的实时监视
		\inmodule ASERStudio

		ASERResourceMoniter是一个单例类，提供对ASER官方资源和项目资源的实时监视功能。
		当项目资源发生变化时，ASERResourceMoniter会自动更新资源列表，通过相关的函数，
		用户可以获取实时更新的项目中可用的资源的名称，包括背景、角色立绘、音乐和音效等。

		\note 这类实际上从2.0.0开始提供，但此前因划分到插件非API部分的YSS命名空间而未提供文档支持。
		在2.4.0之前，其为ASERStudio::YSS::ResourceMoniter，提供与2.4.0版本完全等价的功能。
	*/

	/*!
		\since ASERStudio 2.4.0
		
		return ASERResourceMoniter的单例实例
	*/
	ASERResourceMoniter* ASERResourceMoniter::getInstance() {
		if (!ASERResourceMoniterPrivate::Instance) {
			ASERResourceMoniterPrivate::Instance = new ASERResourceMoniter();
		}
		return ASERResourceMoniterPrivate::Instance;
	}

	/*!
		\since ASERStudio 2.4.0

		构造函数。
	*/
	ASERResourceMoniter::ASERResourceMoniter() {
		d = new ASERResourceMoniterPrivate();
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


	/*!
		\since ASERStudio 2.4.0
		析构函数。
	*/
	ASERResourceMoniter::~ASERResourceMoniter() {
		delete d;
	}

	/*!
		\since ASERStudio 2.4.0
		return ASER官方资源的标准配置文件所在路径

		\warning 这个函数只在Windows平台上有效。其他平台上会返回一个无效路径。
	*/
	QString ASERResourceMoniter::getASERStandardConfigPath() {
		return QDir::homePath() + "/AppData/LocalLow/Gradus/ASE-Remake";
	}

	/*!
		\since ASERStudio 2.4.0

		return 当前项目中可用的背景资源名称列表，包括项目资源和ASER官方资源。资源名称不包含文件扩展名。
	*/
	QStringList ASERResourceMoniter::getBackground() {
		return d->Backgrounds + d->OfficialBackgrounds;
	}

	/*!
		\since ASERStudio 2.4.0

		return 当前项目中可用的角色立绘资源名称列表，包括项目资源和ASER官方资源。资源名称不包含文件扩展名。

		\note 较为特殊的是，ASERStudio未按ASE-Remake的实现分别独立处理普通角色立绘
		与Spine动态立绘。因此如果动态立绘与普通立绘有重复，则动态立绘的结果将覆盖普通立绘的结果。
		这是实现错误，但目前不打算修正。
	*/
	QStringList ASERResourceMoniter::getCharacters() {
		return d->CharaDiffs.keys() + d->OfficialCharaDiffs.keys();
	}

	/*!
		\since ASERStudio 2.4.0
		\a chara 角色名称

		return 给定角色的立绘差分资源名称列表，包括项目资源和ASER官方资源。资源名称不包含文件扩展名。
	*/
	QStringList ASERResourceMoniter::getCharaDiff(const QString& chara) {
		return d->CharaDiffs.value(chara) + d->OfficialCharaDiffs.value(chara);
	}

	/*!
		\since ASERStudio 2.4.0

		return 当前项目中可用的音乐资源名称列表，包括项目资源和ASER官方资源。资源名称不包含文件扩展名。
	*/
	QStringList ASERResourceMoniter::getMusic() {
		return d->Musics + d->OfficialMusics;
	}

	/*!
		\since ASERStudio 2.4.0

		return 当前项目中可用的音效资源名称列表，包括项目资源和ASER官方资源。资源名称不包含文件扩展名。
	*/
	QStringList ASERResourceMoniter::getSoundEffect() {
		return d->SoundEffects + d->OfficialSoundEffects;
	}

	/*!
		\since ASERStudio 2.4.0
		\a path 项目路径

		更改当前监视的项目路径。调用此函数后，ASERResourceMoniter将开始监视新的项目路径，并更新资源列表以反映新路径下的资源。
	*/
	void ASERResourceMoniter::changeProjectPath(const QString& path) {
		d->Watcher->removePaths(d->Watcher->directories());
		d->Watcher->addPath(path + "/Resources");
		d->ProjectPath = path;
		refresh();
	}

	/*!
		\since ASERStudio 2.4.0
		\a types 资源类型标志

		刷新资源列表。根据提供的资源类型标志，ASERResourceMoniter将检查相应类型的资源目录，
		并更新资源列表以反映当前项目路径下的资源状态。
	*/
	void ASERResourceMoniter::refresh(ResourceTypes types) {
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