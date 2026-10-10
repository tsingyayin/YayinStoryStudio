#include <QtCore/qdir.h>
#include <General/Log.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include "FileServer/DefaultTextEdit.h"
#include "FileServer/YSSPEditor.h"
#include "FileTemplate/SimpleFileTemplate.h"
#include "LangServer/IniLangServer.h"
#include "LangServer/JsonLang.h"
#include "LangServer/TomlLangServer.h"
#include "LangServer/XmlLangServer.h"
#include "LangServer/YamlLangServer.h"
#include "Plugin_YSSFileExt.h"
namespace YSSFileExt {
	YSSFileExtTranslator::YSSFileExtTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "YSSFileExt")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/zh_CN.json");
		addLangFilePath(zh_TW, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/zh_TW.json");
		addLangFilePath(en, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/en.json");
		addLangFilePath(ja, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/ja.json");
		addLangFilePath(jp_less_loanword, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/jp_less_loanword.json");
		addLangFilePath(ko, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/ko.json");
		addLangFilePath(ru, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/ru.json");
		addLangFilePath(de, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/de.json");
		addLangFilePath(fr, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssfileext/i18n/fr.json");
	}
}
Plugin_YSSFileExt::Plugin_YSSFileExt() : YSSCore::Editor::EditorPlugin("cn.yxgeneral.yayinstorystudio.plugin.yssfileext") {
	setPluginVersion(Compiled_VIAPI_Version); // YSSFileExt uses the same version as Visindigo API version
	setPluginName("YSS File Extensions");
	setPluginAuthor({ "Tsing Yayin" });
}

void Plugin_YSSFileExt::onPluginEnable() {
	// Migrate the config of the legacy plugin ID to the current one, if it exists.
	const QString legacyPluginID = "cn.yxgeneral.weavestudio.yssfileext";
	const QDir legacyFolder = VIPLM->getPluginFolder(legacyPluginID, Visindigo::General::Plugin::LoadType::FromDisk);
	const QString legacyConfigPath = legacyFolder.filePath("config.json");
	if (Visindigo::Utility::FileUtility::isFileExist(legacyConfigPath)) {
		const QString configPath = getPluginFolder().filePath("config.json");
		const Visindigo::Utility::FileOperation::ErrorCode copyResult =
			Visindigo::Utility::FileOperation::copyFile(legacyConfigPath, configPath, true, true);
		if (copyResult != Visindigo::Utility::FileOperation::Success) {
			vgWarningF << "Failed to migrate config from legacy plugin ID" << legacyPluginID
				<< ", error:" << Visindigo::Utility::FileOperation::errorCodeName(copyResult);
		}
		else {
			reloadPluginConfig();
			// The migration is one-shot: remove the legacy folder so it won't be migrated again.
			const Visindigo::Utility::FileOperation::ErrorCode removeResult =
				Visindigo::Utility::FileOperation::deleteDir(legacyFolder.absolutePath(), false);
			if (removeResult != Visindigo::Utility::FileOperation::Success) {
				vgWarningF << "Failed to remove legacy plugin folder" << legacyFolder.absolutePath()
					<< ", error:" << Visindigo::Utility::FileOperation::errorCodeName(removeResult);
			}
		}
	}
	registerPluginModule(new YSSFileExt::YSSFileExtTranslator(this));
	registerLangServer(new YSSFileExt::JsonLangServer(this));
	registerLangServer(new YSSFileExt::YamlLangServer(this));
	registerLangServer(new YSSFileExt::XmlLangServer(this));
	registerLangServer(new YSSFileExt::TomlLangServer(this));
	registerLangServer(new YSSFileExt::IniLangServer(this));
	registerFileServer(new YSSFileExt::YSSPFileServer(this));
	registerFileServer(new YSSFileExt::DefaultTextEdit(this));
	// 新建文件模板（空文件）
	registerFileTemplateProvider(new YSSFileExt::SimpleFileTemplateProvider(this,
		"YSS File Extension JSON File Template Provider", "YSSFileExt_JsonFileTemplate",
		"YSSFileExtJsonFile",
		VITRL("YSSFileExt::fileProvider.json.name"),
		VITRL("YSSFileExt::fileProvider.json.description"),
		QStringList({ VITRL("YSSFileExt::fileProvider.json.tags") }),
		"json"));
	registerFileTemplateProvider(new YSSFileExt::SimpleFileTemplateProvider(this,
		"YSS File Extension YAML File Template Provider", "YSSFileExt_YamlFileTemplate",
		"YSSFileExtYamlFile",
		VITRL("YSSFileExt::fileProvider.yaml.name"),
		VITRL("YSSFileExt::fileProvider.yaml.description"),
		QStringList({ VITRL("YSSFileExt::fileProvider.yaml.tags") }),
		"yaml"));
	registerFileTemplateProvider(new YSSFileExt::SimpleFileTemplateProvider(this,
		"YSS File Extension XML File Template Provider", "YSSFileExt_XmlFileTemplate",
		"YSSFileExtXmlFile",
		VITRL("YSSFileExt::fileProvider.xml.name"),
		VITRL("YSSFileExt::fileProvider.xml.description"),
		QStringList({ VITRL("YSSFileExt::fileProvider.xml.tags") }),
		"xml"));
	registerFileTemplateProvider(new YSSFileExt::SimpleFileTemplateProvider(this,
		"YSS File Extension TOML File Template Provider", "YSSFileExt_TomlFileTemplate",
		"YSSFileExtTomlFile",
		VITRL("YSSFileExt::fileProvider.toml.name"),
		VITRL("YSSFileExt::fileProvider.toml.description"),
		QStringList({ VITRL("YSSFileExt::fileProvider.toml.tags") }),
		"toml"));
	registerFileTemplateProvider(new YSSFileExt::SimpleFileTemplateProvider(this,
		"YSS File Extension INI File Template Provider", "YSSFileExt_IniFileTemplate",
		"YSSFileExtIniFile",
		VITRL("YSSFileExt::fileProvider.ini.name"),
		VITRL("YSSFileExt::fileProvider.ini.description"),
		QStringList({ VITRL("YSSFileExt::fileProvider.ini.tags") }),
		"ini"));
}

void Plugin_YSSFileExt::onPluginDisable() {
}