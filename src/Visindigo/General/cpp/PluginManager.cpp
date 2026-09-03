#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlist.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include "General/Log.h"
#include "General/Plugin.h"
#include "General/PluginManager.h"
#include "General/PluginModule.h"
#include "General/private/Plugin_p.h"
#include "General/VIApplication.h"
#include "Utility/FileUtility.h"
#include "Utility/JsonConfig.h"

using IDString = QString;
using NameString = QString;

VI_DECLARE_LOGGER(VIPluginManager, VIPM, Visindigo::General);

namespace Visindigo::General {
	class PluginManagerPrivate {
		struct ExternalLibrary{
			QString libPath;
			QString libName;
			QLibrary* lib = nullptr;
			QString md5;
		};

		struct PluginManageData {
			IDString id;
			QString libPath;
			QDir binaryFolder;

			QString metaPath;
			QLibrary* dll = nullptr;
			Visindigo::Utility::JsonConfig meta;
			__VisindigoPluginMain entryPoint = nullptr;
			Plugin* plugin = nullptr;

			quint32 priority = 0;

			PluginManager::LoadPluginResult loadResult = PluginManager::LoadPluginResult::Unknown;
			PluginManager::PluginState state = PluginManager::PluginState::Unknown;

			QStringList getDependencies() const {
				if (meta.isEmpty("Dependencies")) return {};
				return meta.getStringList("Dependencies");
			}
			QStringList getDependLibs() const {
				if (meta.isEmpty("DependLibs")) return {};
				return meta.getStringList("DependLibs");
			}
		};
		friend class PluginManager;
	protected:
		// setPluginLoadPath / setPluginEntryPoint 注入的"随应用分发/打包"插件源：
		// 可用库路径 dlopen(EntryPoint 为空)，或已随进程加载的入口函数指针(LibPath 为空)
		
		static PluginManager* Instance;
		bool loaded = false;
		QMap<IDString, PluginManageData> PluginManage;
		QMap<QString, ExternalLibrary> ExternalLibs;

		QList<IDString> LoadPriorityList;
		QList<IDString> DeactivatedList;
		
		QMap<IDString, QString> PluginTypeDescriptions;
		QMap<IDString, QString> PluginTypeNames;
		QMap<IDString, QString> PluginModuleTypeDescriptions;
		QMap<IDString, QString> PluginModuleTypeNames;

		static QStringList recursionGetAllDll(const QString& path) {
			return Visindigo::Utility::FileUtility::fileFilter(path, { "*.vpl" }, true);
		}

		void parseMetaInfo(PluginManageData data, const QString& metaJsonPath, const QString& binaryPath) {
			auto jsonStr = Visindigo::Utility::FileUtility::readAll(metaJsonPath);
			if (jsonStr.isEmpty()) {
				VIPM->error() << "Failed to read plugin meta json file: " << metaJsonPath << ", IGNORE this plugin.";
				return;
			}
			auto json = Visindigo::Utility::JsonConfig();
			if (json.parse(jsonStr).error != QJsonParseError::NoError) {
				VIPM->error() << "Failed to parse plugin meta json file: " << metaJsonPath << ", IGNORE this plugin.";
				return;
			}
			auto id = json.getString("ID");
			if (id.isEmpty()) {
				VIPM->error() << "Plugin meta json file: " << metaJsonPath << " has no ID, IGNORE this plugin.";
				return;
			}
			auto regex = QRegularExpression(R"(^[a-zA-Z][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9_]*)*$)");
			if (not regex.match(id).hasMatch()) {
				VIPM->error() << "Plugin ID \"" << id << "\" is invalid. It must follow the reverse domain name convention "
					"and only contain letters, numbers (not at the beginning of each segment), and underscores.";
				return;
			}
			if (PluginManage.contains(id)) {
				VIPM->error() << "Plugin ID \"" << id << "\" is already loaded. IGNORE this plugin.";
				return;
			}
			data.id = id;
			data.metaPath = metaJsonPath;
			data.meta = json;
			data.binaryFolder = QDir(binaryPath);
			PluginManage.insert(id, data);
		}

		void autoScanPlugins() {
			VIPM->notice() << "Auto scanning plugins in folder: " << VIApplication::getInstance()->getEnvConfig(VIApplication::PluginFolderPath).toString();
			auto pluginPaths = recursionGetAllDll(VIApplication::getInstance()->getEnvConfig(VIApplication::PluginFolderPath).toString());
			for (auto path : pluginPaths) {
				auto data = PluginManageData();
				data.libPath = path;
				auto binaryFolder = QFileInfo(path).absolutePath();
				auto jsonPath = path + ".json";
				parseMetaInfo(data, jsonPath, binaryFolder);
			}
		}

		void derivePriorityList() {
			// NOTICE: this step does not care about whether the dependencies are all present.
			for (auto id : PluginManage.keys()) {
				auto deps = PluginManage[id].getDependencies();
				for (auto dep : deps) {
					if (PluginManage.contains(dep)) {
						PluginManage[dep].priority++;
					}
				}
			}
			// Sort by priority (descending, higher priority first)
			LoadPriorityList = PluginManage.keys();
			std::sort(LoadPriorityList.begin(), LoadPriorityList.end(), [this](const IDString& a, const IDString& b) {
				return PluginManage[a].priority > PluginManage[b].priority;
				});
			for (auto id : LoadPriorityList) {
				VIPM->notice() << "Plugin \"" << id << "\" priority: " << PluginManage[id].priority;
			}
		}

		PluginManager::LoadPluginResult loadExternalLibs(PluginManageData& data) {
			auto dependLibs = data.getDependLibs();
			QList<QLibrary*> loadedLibs;
			QStringList newLibs;
			for (auto lib : dependLibs) {
				QString absLibPath = data.binaryFolder.absoluteFilePath(lib);
				QString fileName = QFileInfo(absLibPath).fileName();
				if (not ExternalLibs.contains(fileName)) {
					ExternalLibrary extLib;
					extLib.libPath = absLibPath;
					extLib.libName = fileName;
					extLib.lib = new QLibrary(extLib.libPath);
					extLib.md5 = Visindigo::Utility::FileUtility::getFileMD5(absLibPath);
					if (not extLib.lib->load()) {
						VIPM->error() << "Failed to load external library: " << extLib.libPath << ", IGNORE this plugin.";
						data.loadResult = PluginManager::LoadPluginResult::InvalidDependLib;
						data.state = PluginManager::PluginState::Deactivated;
						VIPM->error() << "Unloading all previously loaded external libraries for this plugin.";
						for (auto lib : loadedLibs) {
							lib->unload();
							lib->deleteLater();
						}
						for (auto libName : newLibs) {
							ExternalLibs.remove(libName);
						}
						return PluginManager::LoadPluginResult::InvalidDependLib;
					}
					loadedLibs.append(extLib.lib);
					newLibs.append(extLib.libName);
					ExternalLibs.insert(fileName, extLib);
					VIPM->notice() << "Loaded external library: " << extLib.libPath;
				}
				else {
					auto previousLib = ExternalLibs[fileName];
					if (previousLib.libPath == absLibPath) {
						VIPM->notice() << "External library: " << previousLib.libPath << " already loaded, skip.";
						continue;
					}
					if (previousLib.md5 == Visindigo::Utility::FileUtility::getFileMD5(absLibPath)) {
						VIPM->notice() << "External library: " << absLibPath << " already loaded, previous path: " << previousLib.libPath <<
							" Although have different path, but the MD5 is the same.";
						continue;
					}
					VIPM->error() << "External library: " << absLibPath << " already loaded, previous path: " << previousLib.libPath <<
						" and the MD5 is different. This may cause conflict. IGNORE this plugin.";
					data.loadResult = PluginManager::LoadPluginResult::DependLibConflict;
					data.state = PluginManager::PluginState::Deactivated;
					VIPM->error() << "Unloading all previously loaded external libraries for this plugin.";
					for (auto lib : loadedLibs) {
						lib->unload();
						lib->deleteLater();
					}
					for (auto libName : newLibs) {
						ExternalLibs.remove(libName);
					}
					return PluginManager::LoadPluginResult::DependLibConflict;
				}
			}
			return PluginManager::LoadPluginResult::Success;
		}

		__VisindigoPluginMain getPluginEntryPoint(PluginManageData& data) {
			if (data.entryPoint != nullptr) {
				return data.entryPoint;
			}
			data.dll = new QLibrary(data.libPath);
			VIPM->debug() << "Loading plugin library: " << data.libPath;
			if (not data.dll->load()) {
				VIPM->error() << "Failed to load plugin library: " << data.libPath << ", IGNORE this plugin.";
				data.loadResult = PluginManager::LoadPluginResult::InvalidPluginBinary;
				data.state = PluginManager::PluginState::Deactivated;
				return nullptr;
			}
			auto entryPoint = (__VisindigoPluginMain)data.dll->resolve(Visindigo_PluginMain_Function_Name);
			if (entryPoint == nullptr) {
				VIPM->error() << "Failed to find entry point \"__visindigo_plugin_main\" in plugin library: " << data.libPath << ", IGNORE this plugin.";
				data.loadResult = PluginManager::LoadPluginResult::EntryPointNotFound;
				data.state = PluginManager::PluginState::Deactivated;
				return nullptr;
			}
			data.entryPoint = entryPoint;
			return entryPoint;
		}

		void loadAllPlugin() {
			for (auto pluginID : LoadPriorityList) {
				auto& manageData = PluginManage[pluginID]; // re-check contains is not necessary, (i think)
				VIPM->notice() << "Loading plugin: " << pluginID;
				if (DeactivatedList.contains(pluginID)) {
					manageData.loadResult = PluginManager::LoadPluginResult::Deactivated;
					manageData.state = PluginManager::PluginState::Deactivated;
					VIPM->notice() << "Plugin: " << pluginID << " is deactivated manually, skip.";
					continue;
				}
				bool dependencyFailed = false;
				for (auto dep : manageData.getDependencies()) {
					if (not PluginManage.contains(dep)) {
						manageData.loadResult = PluginManager::LoadPluginResult::DependencyNotFound;
						manageData.state = PluginManager::PluginState::Deactivated;
						VIPM->error() << "Plugin \"" << pluginID << "\" dependency \"" << dep << "\" not found. IGNORE this plugin.";
						dependencyFailed = true;
					}
					auto& depData = PluginManage[dep];
					if (depData.loadResult != PluginManager::LoadPluginResult::Success) {
						manageData.loadResult = PluginManager::LoadPluginResult::DependencyDeactivated;
						manageData.state = PluginManager::PluginState::Deactivated;
						VIPM->error() << "Plugin \"" << pluginID << "\" dependency \"" << dep << "\" load failed or deactivated. IGNORE this plugin.";
						dependencyFailed = true;
					}
				}
				if (dependencyFailed) {
					continue;
				}
				if (loadExternalLibs(manageData) != PluginManager::LoadPluginResult::Success) {
					continue;
				}
				auto entryPoint = getPluginEntryPoint(manageData);
				if (entryPoint == nullptr) {
					VIPM->error() << "Failed to get plugin entry point for plugin: " << pluginID << ", IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::EntryPointNotFound;
					manageData.state = PluginManager::PluginState::Deactivated;
					continue;
				}
				Plugin* instance = nullptr;
				try {
					instance = entryPoint();
				} catch (...) {
					VIPM->error() << "Failed to create plugin instance for plugin: " << pluginID << ", IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::ConstructorError;
					manageData.state = PluginManager::PluginState::Deactivated;
				}
				if (instance == nullptr) {
					VIPM->error() << "Plugin entry point returned nullptr for plugin: " << pluginID << ", IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::ConstructorError;
					manageData.state = PluginManager::PluginState::Deactivated;
					continue;
				}
				if (instance->getPluginID() != pluginID) {
					VIPM->error() << "Plugin ID mismatch for plugin: " << pluginID << ". The plugin instance reports ID: " << instance->getPluginID() << ". IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::MetadataNotSame;
					manageData.state = PluginManager::PluginState::Deactivated;
					delete instance;
					continue;
				}
				if (not Version::isCompatibleABIVersion(Version::getABIVersion(), instance->getPluginABIVersion())) {
					VIPM->error() << "Plugin \"" << pluginID << "\" ABI version " << instance->getPluginABIVersion().toString() <<
						" is not compatible with application ABI version " << Version::getABIVersion().toString() << ". IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::IncompatibleABI;
					manageData.state = PluginManager::PluginState::Deactivated;
					delete instance;
					continue;
				}
				if (not Version::isCompatibleAPIVersion(VIApplication::getInstance()->getMainPlugin()->getPluginAPIVersion(), instance->getPluginAPIVersion())) {
					VIPM->error() << "Plugin \"" << pluginID << "\" API version " << instance->getPluginAPIVersion().toString() <<
						" is not compatible with application API version " << VIApplication::getInstance()->getMainPlugin()->getPluginAPIVersion().toString() << ". IGNORE this plugin.";
					manageData.loadResult = PluginManager::LoadPluginResult::IncompatibleAPI;
					manageData.state = PluginManager::PluginState::Deactivated;
					delete instance;
					continue;
				}
				manageData.loadResult = PluginManager::LoadPluginResult::Success;
				manageData.state = PluginManager::PluginState::InstanceCreated;
				manageData.plugin = instance;
				VIPM->success() << "Plugin \"" << pluginID << "\" loaded successfully, will be enabled later.";
			}
		}
	};

	PluginManager* PluginManagerPrivate::Instance = nullptr;
	/*!
		\class Visindigo::General::PluginManager
		\inheaderfile General/PluginManager.h
		\inmodule Visindigo
		\ingroup VIPlugin
		\brief 此类为Visindigo提供插件管理器.
		\since Visindigo 0.13.0

		PluginManager负责加载和管理插件。

		此类中有几个函数是由Visindigo::General::VIApplication在恰当时机自动调用的，
		一般不需要手动调用这些函数，除非你非常清楚你在做什么。
	*/

	/*!
		\fn void Visindigo::General::PluginManager::pluginLoaded(Plugin* plugin)
		\since Visindigo 0.13.0

		\a plugin 已加载的插件对象指针。

		当插件被成功加载时发出此信号。
	*/

	/*!
		\fn void Visindigo::General::PluginManager::pluginEnabled(Plugin* plugin)
		\since Visindigo 0.13.0

		\a plugin 已启用的插件对象指针。

		当插件被成功启用时发出此信号。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 为父对象。
		构造PluginManager对象。
	*/
	PluginManager::PluginManager(QObject* parent) : QObject(parent) {
		VIPM->notice() << "Initializing";
		d = new PluginManagerPrivate();
		VIPM->success() << "Done";
	}

	/*!
		\since Visindigo 0.13.0
		析构PluginManager对象，卸载所有插件。一般来说，没有任何情况需要手动析构此对象。PluginManager应该与使用它的应用程序有一致的生命周期。
	*/
	PluginManager::~PluginManager() {
		unloadAllPlugin();
		delete d;
	}

	/*!
		\since Visindigo 0.13.0

		return PluginManager单例对象指针。单例不存在时自动创建，是获得PluginManager对象的唯一途径。

		\sa VISPM
	*/
	PluginManager* PluginManager::getInstance() {
		if (PluginManagerPrivate::Instance == nullptr) {
			PluginManagerPrivate::Instance = new PluginManager();
		}
		return PluginManagerPrivate::Instance;
	}

	/*!
			\since Visindigo 0.17.0
			\a libFilePath 插件库文件（绝对路径）。
			\a metaJsonFilePath 该插件的元数据 json（含 "ID"、"Dependencies"；可为文件路径或 qrc ":/…"）。
			\a pluginBinaryFolderPath 插件二进制文件夹路径（可选，若为空则使用 libFilePath 所在目录）。

			手动指定一个插件库文件和其元数据json文件的路径，供PluginManager在loadAllPlugin()时加载。此函数必须在loadAllPlugin()之前调用，否则会被忽略。

			如果这个路径最终会落到插件将自动扫描的目录内，则对此函数的调用无效。请参见VIApplication::EnvKey::PluginFolderPath。
	*/
	void PluginManager::addPluginLoadPath(const QString& libFilePath, const QString& metaJsonFilePath, const QString& pluginBinaryFolderPath) {
		if (d->loaded) {
			VIPM->warning() << "setPluginLoadPath() must be called before loadAllPlugin(); This call is ignored.";
			return;
		}
		if (Utility::FileUtility::isPathInDir(libFilePath, VIApplication::getInstance()->getEnvConfig(VIApplication::PluginFolderPath).toString())) {
			VIPM->warning() << "The plugin library path" << libFilePath << "is in the auto-scanned plugin folder. This call is ignored.";
			return;
		}
		auto data = PluginManagerPrivate::PluginManageData();
		data.libPath = libFilePath;
		auto binaryFolder = pluginBinaryFolderPath;
		if (binaryFolder.isEmpty()) {
			binaryFolder = QFileInfo(libFilePath).absolutePath();
		}
		d->parseMetaInfo(data, metaJsonFilePath, binaryFolder);
	}

	/*!
		\since Visindigo 0.17.0
		\a entryPoint 插件入口函数指针(等价于磁盘插件经 resolve("VisindigoPluginMain") 得到的入口)。
		       用于插件库已随进程加载(如 Android 随 APK lib/<abi> 由主程序 DT_NEEDED 载入)、无法再 dlopen 的场景。
		\a metaJsonFilePath 该插件的元数据 json(含 ID/Dependencies；可为文件路径或 qrc ":/…")。
		\a pluginBinaryFolderPath 插件二进制文件夹路径(可选，若为空则使用 程序文件 所在目录）

		手动指定一个插件入口函数指针和其元数据json文件的路径，供PluginManager在loadAllPlugin()时加载。此函数必须在loadAllPlugin()之前调用，否则会被忽略。
	*/
	void PluginManager::addPluginEntryPoint(__VisindigoPluginMain entryPoint, const QString& metaJsonFilePath, const QString& pluginBinaryFolderPath) {
		if (d->loaded) {
			VIPM->warning() << "setPluginEntryPoint() must be called before loadAllPlugin(); This call is ignored.";
			return;
		}
		if (entryPoint == nullptr) {
			VIPM->warning() << "setPluginEntryPoint() called with nullptr entryPoint; This call is ignored.";
			return;
		}
		auto data = PluginManagerPrivate::PluginManageData();
		data.entryPoint = entryPoint;
		auto binaryFolder = pluginBinaryFolderPath;
		if (binaryFolder.isEmpty()) {
			binaryFolder = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
		}
		d->parseMetaInfo(data, metaJsonFilePath, binaryFolder);
	}

	/*!
		\since Visindigo 0.13.0
		扫描并加载插件到内存里，但不启用它们。

		此函数会扫描Visindigo::General::VIApplication::EnvKey::PluginFolderPath
		指定的目录下的所有插件，并根据插件的依赖关系决定加载顺序。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。

		\note 这个函数不能被多次调用，重复调用会被忽略。
	*/
	void PluginManager::loadAllPlugin() {
		if (d->loaded) {
			VIPM->warning() << "Plugins have already been loaded, this operation will be ignored.";
			return;
		}
		d->loaded = true;
		d->autoScanPlugins();
		d->derivePriorityList();
		d->DeactivatedList = VIApp->getMainPlugin()->getPluginConfig()->getStringList("Plugins.Deactivated");
		d->loadAllPlugin();
	}

	/*!
		\since Visindigo 0.13.0
		测试所有已加载的插件。这调用所有插件的Visindigo::General::Plugin::onTest()函数。

		如果插件未启用测试，则跳过该插件的测试。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果需要手动调用，请安排在applicationInitAllPlugin()之后调用此函数。
	*/
	void PluginManager::testAllPlugin() {
		for (auto plugin : getEnabledPlugins()) {
			if (plugin->isTestEnable()) {
				VIPM->notice() << "Testing plugin" << plugin->getPluginName();
				plugin->onTest();
				LoggerManager::getInstance()->finalSave();
			}
			else {
				VIPM->notice() << "Plugin" << plugin->getPluginName() << "test is disabled, skip.";
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		在所有插件都被启用后，调用所有插件的Visindigo::General::Plugin::onApplicationInit()函数，进行应用程序初始化工作。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果需要手动调用，请安排在enableAllPlugin()之后调用此函数。
	*/
	void PluginManager::applicationInitAllPlugin() {
		for (auto plugin : getEnabledPlugins()) {
			VIPM->notice() << "Plugin" << plugin->getPluginName() << "handling application init";
			plugin->onApplicationInit();
		}
	}

	/*!
		\since Visindigo 0.13.0
		禁用所有已启用的插件。请在调用enableAllPlugin()之后调用此函数。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果在没有启用插件的情况下调用此函数，则不会有任何效果。
	*/
	void PluginManager::disableAllPlugin() {
		for (auto rit = d->LoadPriorityList.rbegin(); rit != d->LoadPriorityList.rend(); ++rit) {
			auto pluginID = *rit;
			auto& manageData = d->PluginManage[pluginID];
			if (manageData.state == PluginManager::PluginState::Enabled) {
				VIPM->notice() << "Disabling plugin: " << pluginID;
				manageData.plugin->onPluginDisable();
				manageData.state = PluginManager::PluginState::Disabled;
				VIPM->success() << "Plugin \"" << pluginID << "\" disabled successfully.";
			}
		}
	}

	/*!
		\since Visindigo 0.17.0
		卸载所有插件，释放所有插件占用的资源。请在调用disableAllPlugin()之后调用此函数。
	*/
	void PluginManager::unloadAllPlugin() {
		for (auto rit = d->LoadPriorityList.rbegin(); rit != d->LoadPriorityList.rend(); ++rit) {
			auto pluginID = *rit;
			auto& manageData = d->PluginManage[pluginID];
			if (manageData.plugin != nullptr) {
				VIPM->notice() << "Unloading plugin: " << pluginID;
				delete manageData.plugin;
				manageData.plugin = nullptr;
				manageData.state = PluginManager::PluginState::Unknown;
				VIPM->success() << "Plugin \"" << pluginID << "\" unloaded successfully.";
			}
			if (manageData.dll != nullptr) {
				manageData.dll->unload();
				delete manageData.dll;
				manageData.dll = nullptr;
			}
		}
		for (auto& extLib : d->ExternalLibs) {
			if (extLib.lib != nullptr) {
				extLib.lib->unload();
				delete extLib.lib;
				extLib.lib = nullptr;
			}
		}
		d->ExternalLibs.clear();
		d->PluginManage.clear();
		d->LoadPriorityList.clear();
		d->DeactivatedList.clear();
	}
	/*!
		\since Visindigo 0.13.0

		return 已加载的插件数量。这是被正确识别且加载到内存的插件动态链接库的数量。

		根据插件加载的成功情况，插件在内存中的关系遵循如下规律：
		\list
		\li 1. 如果插件的metajson未被正确识别，则根本不会触发对动态链接库的加载尝试。
		\li 2. 如果在加载插件的动态链接库时，插件主函数返回的插件实例为nullptr，
		或ID与metajson不符，则立即析构该插件实例（如果可以），并立即卸载该动态链接库。
		\li 3. 在启用插件时，如果发生异常，则立即调用该插件的禁用函数，并保持该插件处于未启用状态，但插件仍然保留在内存中。
		\endlist

		\sa getEnabledPluginCount()
	*/
	qint32 PluginManager::getLoadedPluginCount() const {
		auto rtn = 0;
		for (auto pluginID : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[pluginID];
			if (manageData.loadResult == PluginManager::LoadPluginResult::Success) {
				rtn++;
			}
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.13.0

		return 已启用的插件数量。这是被启用并可以使用的插件对象的数量。

		\sa getLoadedPluginCount()
	*/
	qint32 PluginManager::getEnabledPluginCount() const {
		auto rtn = 0;
		for (auto pluginID : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[pluginID];
			if (manageData.state == PluginManager::PluginState::Enabled) {
				rtn++;
			}
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.13.0

		return ID为 \a id 的插件是否被启用。
	*/
	bool PluginManager::isPluginEnable(const QString& id) const {
		if (not d->PluginManage.contains(id)) {
			return false;
		}
		return d->PluginManage.value(id).state == PluginManager::PluginState::Enabled;
	}

	/*!
		\since Visindigo 0.13.0

		return \a plugin 插件是否被启用。
	*/
	bool PluginManager::isPluginEnable(Plugin* plugin) const {
		return isPluginEnable(plugin->getPluginID());
	}

	/*!
		\since Visindigo 0.13.0
		设置ID为 \a id 的插件的 \a deactivate 状态，true为禁用，false为启用。

		被设置为false状态的插件将在加载动态链接库到内存这一步时被跳过，因此也不会启用。
		请注意，如果这插件是其他插件的依赖项，那么依赖它的所有插件都会被连带禁用。

		只要调用这个函数，它就会同步修改主插件配置文件中"Plugins.Deactivated"项的内容，以确保这个设置在下次启动时仍然有效。
		被连带禁用的插件不会出现在这个配置项中，这个配置项永远只包含被手动设置为禁用状态的插件ID。

	*/
	void PluginManager::setPluginDeactivate(const QString& id, bool deactivate) {
		if (not deactivate) {
			d->DeactivatedList.removeAll(id);
		}
		else {
			if (not d->DeactivatedList.contains(id)) {
				d->DeactivatedList.append(id);
			}
		}
		VIApp->getMainPlugin()->getPluginConfig()->setStringList("Plugins.Deactivated", d->DeactivatedList);
	}

	/*!
		\since Visindigo 0.13.0

		return ID为 \a id 的插件是否被手动设置为禁用状态。

		请注意，和禁用状态有关的几个函数都有以下要点：
		\list
		\li 1. 激活状态是个纯配置的项目，和插件是否实际被加载和启用没有关系。
		即使在全部插件都没加载的阶段，也可以设置某个插件的激活状态。它不依赖插件实例，只从ID判断。
		\li 2. 一个插件被配置为禁用状态后，依赖它的所有插件都会被连带设置为禁用状态。
		但连带禁用的插件不会出现在禁用列表中，这个列表永远只含有被手动指定为禁用的插件ID。
		\endlist

		如果需要知道所有被禁用（无论是手动禁用还是连带禁用）的插件ID，
		可以使用getPluginLoadResultByID()函数来判断插件的加载结果是否为Deactivated。
	*/
	bool PluginManager::isPluginDeactivate(const QString& id) const {
		return d->DeactivatedList.contains(id);
	}

	/*!
		\since Visindigo 0.13.0
		\a deactivatedList 被设置为禁用状态的插件ID列表。
		设置禁用状态的插件ID列表。这个列表只包含被手动设置为禁用状态的插件ID，而不包括那些因为依赖关系而被自动禁用的插件ID。

		这个设置会自动进入主插件配置文件的"Plugins.Deactivated"项，以确保这个设置在下次启动时仍然有效。
	*/
	void PluginManager::setDeactivatePluginList(const QStringList& deactivatedList) {
		d->DeactivatedList = deactivatedList;
		VIApp->getMainPlugin()->getPluginConfig()->setStringList("Plugins.Deactivated", d->DeactivatedList);
	}

	/*!
		\since Visindigo 0.13.0
		return 所有被设置为禁用状态的插件ID列表。

		请注意，这只包括被手动设置为禁用状态的插件ID，而不包括那些因为依赖关系而被自动禁用的插件ID。
	*/
	QStringList PluginManager::getDeactivatedPluginIDList() const {
		return d->DeactivatedList;
	}

	/*!
		\since Visindigo 0.13.0
		启用所有已加载但未启用的插件。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果需要手动调用，请安排在loadAllPlugin()之后调用此函数。
	*/
	void PluginManager::enableAllPlugin() {
		for (auto id : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[id];
			if (manageData.loadResult == PluginManager::LoadPluginResult::Success) {
				if (manageData.state == PluginManager::PluginState::InstanceCreated) {
					VIPM->notice() << "Enabling plugin: " << id;
					manageData.plugin->onPluginEnable();
					manageData.state = PluginManager::PluginState::Enabled;
					VIPM->success() << "Plugin \"" << id << "\" enabled successfully.";
				}
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		根据 \a id 获取插件对象指针。如果插件ID不存在或未被正确加载，则返回nullptr。
	*/
	Plugin* PluginManager::getPluginByID(const QString& id) const {
		if (d->PluginManage.contains(id) && 
			d->PluginManage.value(id).state == PluginManager::PluginState::InstanceCreated ||
			d->PluginManage.value(id).state == PluginManager::PluginState::Enabled){
			return d->PluginManage.value(id).plugin;
		}
		return nullptr;
	}

	/*!
		\since Visindigo 0.16.0
		根据 \a id 获取插件二进制文件(.vpl)所在的目录。

		这个目录是插件动态库实际存放的目录，与插件的配置目录(getPluginFolder())不同。
		如果插件ID不存在或未被加载，返回空的QDir。

		\sa Plugin::getPluginBinaryFolder()
	*/
	QDir PluginManager::getPluginBinaryFolder(const QString& id) const {
		if (d->PluginManage.contains(id) && d->PluginManage.value(id).state == PluginManager::PluginState::InstanceCreated ||
			d->PluginManage.value(id).state == PluginManager::PluginState::Enabled) {
			return d->PluginManage.value(id).binaryFolder;
		}
		return QDir();
	}

	/*!
		\since Visindigo 0.13.0
		根据插件名称 \a name 获取插件对象指针列表，其中包括全部具有此名称的插件对象指针。对象必须已经被加载到内存中，无论它们是否被启用。
	*/
	QList<Plugin*> PluginManager::getPluginByName(const QString& name) const {
		QList<Plugin*> plugins;
		for (auto pluginID : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[pluginID];
			if (manageData.state == PluginManager::PluginState::InstanceCreated ||
				manageData.state == PluginManager::PluginState::Enabled) {
				if (manageData.plugin->getPluginName() == name) {
					plugins.append(manageData.plugin);
				}
			}
		}
		return plugins;
	}

	/*!
		\since Visindigo 0.13.0
		根据 \a id 获取插件的加载结果。这个结果反映了插件在加载阶段的状态，包括是否成功加载、是否被禁用、是否缺少依赖等。

		如果插件ID不存在，则返回LoadPluginResult::Unknown。
	*/
	PluginManager::LoadPluginResult PluginManager::getPluginLoadResultByID(const QString& id) const {
		if (d->PluginManage.contains(id)) {
			return d->PluginManage.value(id).loadResult;
		}
		return LoadPluginResult::Unknown;
	}

	/*!
		\since Visindigo 0.13.0
		return 已加载的插件对象列表。这个列表包含所有被正确识别并加载到内存中的插件对象指针，无论它们是否被启用。
	*/
	QList<Plugin*> PluginManager::getLoadedPlugins() const {
		auto rtn = QList<Plugin*>();
		for (auto id : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[id];
			if (manageData.state == PluginManager::PluginState::InstanceCreated ||
				manageData.state == PluginManager::PluginState::Enabled) {
				rtn.append(manageData.plugin);
			}
		}
		return  rtn;
	}

	/*!
		\since Visindigo 0.15.1
		return 已启用的插件对象列表。这个列表包含所有被启用并可以使用的插件对象指针。
	*/
	QList<Plugin*> PluginManager::getEnabledPlugins() const {
		auto rtn = QList<Plugin*>();
		for (auto id : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[id];
			if (manageData.state == PluginManager::PluginState::Enabled) {
				rtn.append(manageData.plugin);
			}
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.13.0
		return 所有插件的加载结果映射。这个映射包含所有已加载插件的ID和对应的加载结果。
	*/
	QMap<QString, PluginManager::LoadPluginResult> PluginManager::getAllPluginLoadResults() const {
		auto rtn = QMap<QString, PluginManager::LoadPluginResult>();
		for (auto id : d->LoadPriorityList) {
			auto& manageData = d->PluginManage[id];
			if (manageData.state == PluginManager::PluginState::InstanceCreated ||
				manageData.state == PluginManager::PluginState::Enabled) {
				rtn[id] = manageData.loadResult;
			}
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.13.0
		\a typeID 插件类型ID，必须是唯一的字符串标识符。
		\a typeName 插件类型名称，用于在用户界面中显示。
		\a description 插件类型描述，用于在用户界面中显示详细信息。

		为插件类型ID \a typeID 设置插件类型名称和描述。这些信息可以在用户界面中显示，以帮助用户识别和理解不同类型的插件。
	*/
	void PluginManager::setPluginTypeDescription(const QString& typeID, const QString& typeName, const QString& description) {
		d->PluginTypeNames[typeID] = typeName;
		d->PluginTypeDescriptions[typeID] = description;
	}

	/*!
		\since Visindigo 0.13.0
		return 根据插件类型ID \a typeID 获取的插件类型名称。如果类型ID不存在，则返回空字符串。
	*/
	QString PluginManager::getPluginTypeName(const QString& typeID) const {
		return d->PluginTypeNames.value(typeID, QString());
	}

	/*!
		\since Visindigo 0.13.0
		return 根据插件类型ID \a typeID 获取的插件类型描述。如果类型ID不存在，则返回空字符串。
	*/
	QString PluginManager::getPluginTypeDescription(const QString& typeID) const {
		return d->PluginTypeDescriptions.value(typeID, QString());
	}

	/*!
		\since Visindigo 0.13.0
		return 所有已注册的插件类型ID列表。这个列表包含所有被注册的插件类型ID字符串。
	*/
	QStringList PluginManager::getAllPluginTypeID() const {
		return d->PluginTypeNames.keys();
	}

	/*!
		\since Visindigo 0.13.0
		\a typeID 插件模块类型ID，必须是唯一的字符串标识符。
		\a typeName 插件模块类型名称，用于在用户界面中显示。
		\a description 插件模块类型描述，用于在用户界面中显示详细信息。

		为插件模块类型ID \a typeID 设置插件模块类型名称和描述。这些信息可以在用户界面中显示，以帮助用户识别和理解不同类型的插件模块。
	*/
	void PluginManager::setPluginModuleTypeDescription(const QString& typeID, const QString& typeName, const QString& description) {
		d->PluginModuleTypeNames[typeID] = typeName;
		d->PluginModuleTypeDescriptions[typeID] = description;
	}

	/*!
		\since Visindigo 0.13.0
		return 根据插件模块类型ID \a typeID 获取的插件模块类型名称。如果类型ID不存在，则返回空字符串。
	*/
	QString PluginManager::getPluginModuleTypeName(const QString& typeID) const {
		return d->PluginModuleTypeNames.value(typeID, QString());
	}

	/*!
		\since Visindigo 0.13.0
		return 根据插件模块类型ID \a typeID 获取的插件模块类型描述。如果类型ID不存在，则返回空字符串。
	*/
	QString PluginManager::getPluginModuleTypeDescription(const QString& typeID) const {
		return d->PluginModuleTypeDescriptions.value(typeID, QString());
	}

	/*!
		\since Visindigo 0.13.0
		return 所有已注册的插件模块类型ID列表。这个列表包含所有被注册的插件模块类型ID字符串。
	*/
	QStringList PluginManager::getAllPluginModuleTypeID() const {
		return d->PluginModuleTypeNames.keys();
	}
}