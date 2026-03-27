#include <QtCore/qstring.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include "../../General/Log.h"
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"
#include "../private/Plugin_p.h"
#include <QtCore/qmap.h>
#include "../PluginManager.h"
#include "../Plugin.h"
#include "../VIApplication.h"
#include <memory>

using IDString = QString;
using NameString = QString;
namespace Visindigo::General {
	class PluginManagerPrivate {
		friend class PluginManager;
	protected:
		static PluginManager* Instance;
		bool loaded = false;
		QStringList DeactivatedPluginIDList;
		QList<Plugin*> Plugins;
		QList<Plugin*> EnabledPlugins;
		QMap<IDString, Plugin*> PluginIDMap;
		QMap<IDString, quint32> PriorityMap;
		QList<IDString> PriorityPlugins;
		QMap<IDString, QString> PluginPathMap;
		QMap<IDString, QLibrary*> Dlls;
		QMap<IDString, QStringList> Dependencies;
		QMap<IDString, Visindigo::General::PluginManager::LoadPluginResult> LoadResults;
		QMap<IDString, QString> PluginTypeDescriptions;
		QMap<IDString, QString> PluginTypeNames;
		QMap<IDString, QString> PluginModuleTypeDescriptions;
		QMap<IDString, QString> PluginModuleTypeNames;
		static QFileInfoList recursionGetAllDll(const QString& path) {
			QDir dir(path);
			QStringList filters;
			filters << "*.vpl";
			dir.setNameFilters(filters);
			QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
			dir = QDir(path);
			QFileInfoList subList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
			for (int i = 0; i < subList.size(); i++) {
				list.append(recursionGetAllDll(subList[i].absoluteFilePath()));
			}
			return list;
		}
	};

	PluginManager* PluginManagerPrivate::Instance = nullptr;
	/*!
		\class Visindigo::General::PluginManager
		\inheaderfile General/PluginManager.h
		\inmodule Visindigo
		\ingroup VIPlugin
		\brief 此类为Visindigo提供插件管理器。
		\since Visindigo 0.13.0

		PluginManager负责加载和管理插件。

		此类中有几个函数是由Visindigo::General::VIApplication在恰当时机自动调用的，
		一般不需要手动调用这些函数，除非你非常清楚你在做什么。

		如果你不想使用Visindigo::General::VIApplication，但又需要插件管理功能，
		你可以手动调用这些函数，但请确保调用顺序正确：
		\list
		\li 1. loadAllPlugin()
		\li 2. enableAllPlugin()
		\li 3. applicationInitAllPlugin()
		\li 4. testAllPlugin()
		\li 5. disableAllPlugin()
		\endlist
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
		d = new PluginManagerPrivate();
		vgSuccessF << "Plugin Manager initialized successfully.";
	}

	/*!
		\since Visindigo 0.13.0
		析构PluginManager对象，卸载所有插件。一般来说，没有任何情况需要手动析构此对象。PluginManager应该与使用它的应用程序有一致的生命周期。
	*/
	PluginManager::~PluginManager() {
		for (int i = 0; i < d->Plugins.size(); i++) {
			delete d->Plugins[i];
		}
		d->Plugins.clear();
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		返回PluginManager单例对象指针。单例不存在时自动创建，是获得PluginManager对象的唯一途径。

		\sa VISPM
	*/
	PluginManager* PluginManager::getInstance() {
		if (PluginManagerPrivate::Instance == nullptr) {
			PluginManagerPrivate::Instance = new PluginManager();
		}
		return PluginManagerPrivate::Instance;
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
			vgWarning << "Plugins have already been loaded, this operation will be ignored.";
			return;
		}
		QString pluginFolder = VIApplication::getInstance()->getEnvConfig(VIApplication::PluginFolderPath).toString();
		vgNoticeF << "Scanning plugins in" << pluginFolder;
		
		QFileInfoList Plugins = PluginManagerPrivate::recursionGetAllDll(pluginFolder);
		for (QFileInfo info : Plugins) {
			QString path = info.absoluteFilePath() + ".json";
			QString jsonStr = Visindigo::Utility::FileUtility::readAll(path);
			Utility::JsonConfig jsonConfig(jsonStr);
			IDString id = jsonConfig.getString("ID");
			if (id.isEmpty()) {
				vgWarning << "The json file in " << info.path() << "does not contain \"ID\" key. IGNORED";
				continue;
			}
			if (d->PluginPathMap.contains(id)){
				vgWarning << "Plugin with meta-id" << id << "has already exist. The plugin in" << info.path() << "will be ignored.";
				continue;
			}
			d->PluginPathMap.insert(id, info.absoluteFilePath());
			if (!d->PriorityMap.contains(id)) {
				d->PriorityMap.insert(id, 0);
			}
			vgMessageF << "Plugin with meta-id" << id << "finded";
			if (!jsonConfig.contains("Dependencies")) {
				continue;
			}
			QStringList dependIndex = jsonConfig.keys("Dependencies");
			for (QString de : dependIndex) {
				IDString dependID = jsonConfig.getString("Dependencies." + de);
				if (!d->PriorityMap.contains(dependID)) {
					d->PriorityMap.insert(dependID, 1);
					d->Dependencies.insert(id, QStringList() << dependID);
				}
				else {
					d->PriorityMap[dependID] += 1;
					d->Dependencies[id].append(dependID);
				}
			}
		}

		vgMessageF << "Determining loading order based on priority";
		d->PriorityPlugins = d->PriorityMap.keys();
		std::sort(d->PriorityPlugins.begin(), d->PriorityPlugins.end(), [this](const QString& a, const QString& b) {
			return d->PriorityMap[a] > d->PriorityMap[b];
			});
		for (int i = 0; i < d->PriorityPlugins.length(); i++) {
			vgMessageF << "Load order:" << d->PriorityPlugins[i] << "[" << i << "]";
		}
		for (QString key : d->PriorityPlugins) {
			d->LoadResults[key] = PluginManager::LoadPluginResult::Unknown;
			if (d->DeactivatedPluginIDList.contains(key)) {
				vgWarning << "Plugin with id" << key << "is in deactivated list, the plugin in will be ignored.";
				d->LoadResults[key] = PluginManager::LoadPluginResult::Deactivated;
				continue;
			}
			QStringList dependList = d->Dependencies.value(key);
			bool dependFailed = false;
			for (IDString depend : dependList) {
				if (!d->PluginIDMap.contains(depend)) {
					vgError << "Failed when load plugin" << key << ", dependency" << depend << "not found!";
					d->LoadResults[key] = PluginManager::LoadPluginResult::DependencyNotFound;
					dependFailed = true;
				}
				else {
					if (d->LoadResults[depend] != PluginManager::LoadPluginResult::Success) {
						if (d->LoadResults[depend] == PluginManager::LoadPluginResult::Deactivated){
							vgError << "Failed when load plugin" << key << ", dependency" << depend << "is deactivated!";
							d->LoadResults[key] = PluginManager::LoadPluginResult::Deactivated;
						}
						else {
							vgError << "Failed when load plugin" << key << ", as dependency" << depend << "failed to load!";
							d->LoadResults[key] = PluginManager::LoadPluginResult::DependencyLoadFailed;
						}
						dependFailed = true;
					}
					else {
						vgMessageF << "Dependency" << depend << "for plugin" << key << "is loaded successfully.";
					}
				}
			}
			if (dependFailed) {
				continue;
			}
			QString path = d->PluginPathMap.value(key);
			QLibrary* hLibrary = new QLibrary(path);
			if (hLibrary->load() == false) {
				vgError << "Failed when load plugin" << key << ", cannot load plugin file into memory!";
				d->LoadResults[key] = PluginManager::LoadPluginResult::InvalidPluginBinary;
				hLibrary->deleteLater();
				continue;
			}
			__VisindigoPluginMain PluginDllMain = (__VisindigoPluginMain)hLibrary->resolve(Visindigo_PluginMain_Function_Name);
			if (PluginDllMain == nullptr) {
				vgError << "Failed when load plugin" << key << ", cannot find entry point VisindigoPluginMain!";
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::EntryPointNotFound;
				hLibrary->deleteLater();
				continue;
			}
			Plugin* plugin = nullptr;
			try {
				plugin = PluginDllMain();
			}
			catch (...) {
				vgError << "Exception occured when load plugin" << key << ", exception has been catched, but may have other impace. RESTART is RECOMMENDED! ";
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::ConstructorError;
				hLibrary->deleteLater();
				return;
			}
			if (plugin == nullptr) {
				vgError << "Failed when init plugin" << key << ", cannot create EditorPlugin Instance!";
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::ConstructorError;
				hLibrary->deleteLater();
				return;
			}
			QString pluginID = plugin->getPluginID();
			if (pluginID != key) {
				vgError << "Failed when init plugin" << key << ", the ID from plugin instance is different from the ID from meta file.";
				delete plugin;
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::MetadataNotSame;
				hLibrary->deleteLater();
				continue;
			}
			bool apiCompatible = Visindigo::General::Version::isCompatibleAPIVersion(
				Visindigo::General::Version::getAPIVersion(), plugin->getPluginAPIVersion()
			);
			if (!apiCompatible) {
				vgError << "Failed when init plugin" << key << ", API incompatible. This plugin use api " << plugin->getPluginAPIVersion()
					<< ", but program is" << Visindigo::General::Version::getAPIVersion();
				delete plugin;
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::IncompatibleAPI;
				hLibrary->deleteLater();
				continue;
			}
			bool abiCompatible = Visindigo::General::Version::isCompatibleABIVersion(
				Visindigo::General::Version::getABIVersion(), plugin->getPluginABIVersion()
			);
			if (!abiCompatible) {
				vgError << "Failed when init plugin" << key << ", ABI incompatible. This plugin use abi " << plugin->getPluginABIVersion()
					<< ", but program is" << Visindigo::General::Version::getABIVersion();
				delete plugin;
				hLibrary->unload();
				d->LoadResults[key] = PluginManager::LoadPluginResult::IncompatibleABI;
				hLibrary->deleteLater();
				continue;
			}
			d->Plugins.append(plugin);
			d->Dlls.insert(plugin->getPluginID(), hLibrary);
			d->PluginIDMap.insert(plugin->getPluginID(), plugin);
			plugin->d->PluginFolder = path;
			d->LoadResults[key] = PluginManager::LoadPluginResult::Success;
			emit pluginLoaded(plugin);
			vgSuccessF << "Plugin" << key << "create instance successfully. Will be enable later";
		}
		for (int i = 0; i < d->PriorityPlugins.size();) {
			IDString id = d->PriorityPlugins.at(i);
			if (d->LoadResults[id] == PluginManager::LoadPluginResult::Success) {
				i++;
			}
			else {
				d->PriorityPlugins.removeAt(i);
			}
		}
		d->loaded = true;
	}

	/*!
		\since Visindigo 0.13.0
		测试所有已加载的插件。这调用所有插件的Visindigo::General::Plugin::onTest()函数。
		
		如果插件未启用测试，则跳过该插件的测试。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果需要手动调用，请安排在applicationInitAllPlugin()之后调用此函数。
	*/
	void PluginManager::testAllPlugin() {
		for (int i = 0; i < d->Plugins.size(); i++) {
			Plugin* plugin = d->Plugins[i];
			if (plugin->isTestEnable()) {
				vgMessageF << "Testing plugin" << plugin->getPluginName();
				plugin->onTest();
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
		for (int i = 0; i < d->Plugins.size(); i++) {
			Plugin* plugin = d->Plugins[i];
			vgMessageF << plugin->getPluginName() << " is handling application init";
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
		// disable in reverse order
		for (int i = d->PriorityPlugins.size() - 1; i >= 0; i--) {
			Plugin* plugin = d->PluginIDMap[d->PriorityPlugins[i]];
			if (isPluginEnable(plugin)){
				try {
					vgMessageF << "Trying to disable plugin" << plugin->getPluginName();
					plugin->onPluginDisable();
				}
				catch (...) {
					vgError << "Failed when disable plugin" << plugin->getPluginName() << ", disable function may not have completed properly. RESTART is RECOMMENDED!";
					continue;
				}
				vgSuccessF << "Plugin" << plugin->getPluginName() << "disabled";
				d->EnabledPlugins.removeAll(plugin);
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		返回已加载的插件数量。这是被正确识别且加载到内存的插件动态链接库的数量。

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
		return d->Plugins.size();
	}

	/*!
		\since Visindigo 0.13.0
		返回已启用的插件数量。这是被启用并可以使用的插件对象的数量。

		\sa getLoadedPluginCount()
	*/
	qint32 PluginManager::getEnabledPluginCount() const {
		return d->EnabledPlugins.size();
	}

	/*!
		\since Visindigo 0.13.0
		检查ID为\a id的插件是否被启用。
	*/
	bool PluginManager::isPluginEnable(const QString& id) const {
		if (d->PluginIDMap.contains(id)) {
			return isPluginEnable(d->PluginIDMap.value(id));
		}
		return false;
	}

	/*!
		\since Visindigo 0.13.0
		检查\a plugin插件是否被启用。
	*/
	bool PluginManager::isPluginEnable(Plugin* plugin) const {
		return d->EnabledPlugins.contains(plugin);
	}

	/*!
		\since Visindigo 0.13.0
		设置ID为\a id的插件的禁用状态，true为禁用，false为启用。

		被设置为false状态的插件将在加载动态链接库到内存这一步时被跳过，因此也不会启用。
		请注意，如果这插件是其他插件的依赖项，那么依赖它的所有插件都会被连带禁用。
		
		只要调用这个函数，它就会同步修改主插件配置文件中"Plugins.Deactivated"项的内容，以确保这个设置在下次启动时仍然有效。
		被连带禁用的插件不会出现在这个配置项中，这个配置项永远只包含被手动设置为禁用状态的插件ID。

	*/
	void PluginManager::setPluginDeactivate(const QString& id, bool deactivate) {
		if (!deactivate) {
			d->DeactivatedPluginIDList.removeAll(id);
		}
		else {
			if (!d->DeactivatedPluginIDList.contains(id)) {
				d->DeactivatedPluginIDList.append(id);
			}
		}
		VIApp->getMainPlugin()->getPluginConfig()->setStringList("Plugins.Deactivated", d->DeactivatedPluginIDList);
	}

	/*!
		\since Visindigo 0.13.0
		检查ID为\a id的插件是否被手动设置为禁用状态。

		请注意，和禁用状态有关的几个函数都有以下要点：
		\list
		\li 1. 激活状态是个纯配置的项目，和插件是否实际被加载和启用没有关系。
		即使在全部插件都没加载的阶段，也可以设置某个插件的激活状态。它不依赖插件实例，只从ID判断。
		\li 2. 一个插件被配置为禁用状态后，依赖它的所有插件都会被连带设置为禁用状态。
		但连带禁用的插件不会出现在禁用列表中，这个列表永远只含有被手动指定为禁用的插件ID。

		如果需要知道所有被禁用（无论是手动禁用还是连带禁用）的插件ID，
		可以使用getPluginLoadResultByID()函数来判断插件的加载结果是否为Deactivated。
	*/
	bool PluginManager::isPluginDeactivate(const QString& id) const {
		return d->DeactivatedPluginIDList.contains(id);
	}

	/*!
		\since Visindigo 0.13.0
		从配置文件中加载被设置为未激活状态的插件ID列表。这个函数会在用户通过
		VIApplication::setMainPlugin()设置主插件时被自动调用，
		以确保在加载插件之前就正确地识别出哪些插件被设置为未激活状态。

		这个配置会自动寄生到主插件的配置文件中，路径为"Plugins.Deactivated"，
		因此你也可以直接编辑配置文件来修改这个列表。
	*/
	void PluginManager::loadDeactivatePluginList() {
		d->DeactivatedPluginIDList = VIApp->getMainPlugin()->getPluginConfig()->getStringList("Plugins.Deactivated");
	}

	/*!
		\since Visindigo 0.13.0
		返回所有被设置为未激活状态的插件ID列表。
		
		请注意，这只包括被手动设置为未激活状态的插件ID，而不包括那些因为依赖关系而被自动禁用的插件ID。
	*/
	QStringList PluginManager::getDeactivatedPluginIDList() const {
		return d->DeactivatedPluginIDList;
	}

	/*!
		\since Visindigo 0.13.0
		启用所有已加载但未启用的插件。

		\warning 一般不手动调用此函数，其调用由Visindigo::General::VIApplication在恰当时机自动完成。
		如果需要手动调用，请安排在loadAllPlugin()之后调用此函数。
	*/
	void PluginManager::enableAllPlugin() {
		for (int i = 0; i < d->PriorityPlugins.size(); i++) {
			Plugin* plugin = d->PluginIDMap[d->PriorityPlugins[i]];
			if (!isPluginEnable(plugin)) {
				try {
					plugin->d->initializePluginFolder(VIApp->getEnvConfig(VIApplication::ConfigPath).toString()+"/plugins");
					plugin->d->setPluginLoadType(Plugin::LoadType::FromDisk);
					vgMessageF << "Trying to enable plugin" << plugin->getPluginName();
					plugin->onPluginEnable();
				}
				catch (...) {
					vgError << "Failed when enable plugin" << plugin->getPluginName() << ", disable function will be called. RESTART is RECOMMENDED!";
					plugin->onPluginDisable();
					continue;
				}
				vgSuccessF << "Plugin" << plugin->getPluginName() << "enabled";
				emit pluginEnabled(plugin);
				d->EnabledPlugins.append(plugin);
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		根据插件ID获取插件对象指针。如果插件ID不存在或未被正确加载，则返回nullptr。
	*/
	Plugin* PluginManager::getPluginByID(const QString& id) const {
		if (d->PluginIDMap.contains(id)) {
			return d->PluginIDMap.value(id);
		}
		return nullptr;
	}

	/*!
		\since Visindigo 0.13.0
		根据插件名称获取插件对象指针。如果插件名称不存在或未被正确加载，则返回nullptr。
	*/
	Plugin* PluginManager::getPluginByName(const QString& name) const {
		for (int i = 0; i < d->Plugins.size(); i++) {
			if (d->Plugins[i]->getPluginName() == name) {
				return d->Plugins[i];
			}
		}
		return nullptr;
	}

	PluginManager::LoadPluginResult PluginManager::getPluginLoadResultByID(const QString& id) const {
		if (d->LoadResults.contains(id)) {
			return d->LoadResults.value(id);
		}
		return LoadPluginResult::Unknown;
	}

	QList<Plugin*> PluginManager::getLoadedPlugins() const {
		return d->Plugins;
	}

	QMap<QString, PluginManager::LoadPluginResult> PluginManager::getAllPluginLoadResults() const {
		return d->LoadResults;
	}

	void PluginManager::setPluginTypeDescription(const QString& typeID, const QString& typeName, const QString& description) {
		d->PluginTypeNames[typeID] = typeName;
		d->PluginTypeDescriptions[typeID] = description;
	}

	QString PluginManager::getPluginTypeName(const QString& typeID) const {
		return d->PluginTypeNames.value(typeID, QString());
	}

	QString PluginManager::getPluginTypeDescription(const QString& typeID) const {
		return d->PluginTypeDescriptions.value(typeID, QString());
	}

	QStringList PluginManager::getAllPluginTypeID() const {
		return d->PluginTypeNames.keys();
	}

	void PluginManager::setPluginModuleTypeDescription(const QString& typeID, const QString& typeName, const QString& description) {
		d->PluginModuleTypeNames[typeID] = typeName;
		d->PluginModuleTypeDescriptions[typeID] = description;
	}

	QString PluginManager::getPluginModuleTypeName(const QString& typeID) const {
		return d->PluginModuleTypeNames.value(typeID, QString());
	}

	QString PluginManager::getPluginModuleTypeDescription(const QString& typeID) const {
		return d->PluginModuleTypeDescriptions.value(typeID, QString());
	}

	QStringList PluginManager::getAllPluginModuleTypeID() const {
		return d->PluginModuleTypeNames.keys();
	}
}