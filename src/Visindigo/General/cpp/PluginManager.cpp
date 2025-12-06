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

namespace Visindigo::General {
	class PluginManagerPrivate {
		friend class PluginManager;
	protected:
		static PluginManager* Instance;
		QList<Plugin*> Plugins;
		QList<Plugin*> EnabledPlugins;
		QMap<QString, Plugin*> PluginIDMap;
		QMap<QString, quint32> PriorityMap;
		QList<QString> PriorityPlugins;
		QMap<QString, QString> PluginPathMap;
		QMap<QString, QLibrary*> Dlls;
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
		\class Visindigo::Editor::PluginManager
		\inmodule Visindigo
		\brief 此类为YayinStoryStudio提供插件管理器。
		\since Visindigo 0.13.0

		PluginManager负责加载和管理插件。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 为父对象。
		构造PluginManager对象。
	*/
	PluginManager::PluginManager(QObject* parent) : QObject(parent) {
		d = new PluginManagerPrivate();
		ySuccessF << "Plugin Manager initialized successfully.";
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

	PluginManager* PluginManager::getInstance() {
		if (PluginManagerPrivate::Instance == nullptr) {
			PluginManagerPrivate::Instance = new PluginManager();
		}
		return PluginManagerPrivate::Instance;
	}

	/*!
		\since Visindigo 0.13.0
		扫描并加载插件，但不启用它们。此函数会扫描./resources/plugins目录下的所有插件，并根据插件的依赖关系决定加载顺序。
		按需调用此函数，然后调用enableAllPlugin()启用插件。
	*/
	void PluginManager::loadAllPlugin() {
		QString pluginFolder = VIApplication::getInstance()->getEnvConfig(VIApplication::PluginFolderPath).toString();
		yNotice << "Scanning plugins in" << pluginFolder;
		
		QFileInfoList Plugins = PluginManagerPrivate::recursionGetAllDll(pluginFolder);
		for (QFileInfo info : Plugins) {
			QString path = info.absoluteFilePath() + ".json";
			QString jsonStr = Visindigo::Utility::FileUtility::readAll(path);
			Utility::JsonConfig jsonConfig(jsonStr);
			QString id = jsonConfig.getString("ID");
			if (id.isEmpty()) {
				yWarning << "The json file in " << info.path() << "does not contain \"ID\" key. IGNORED";
				continue;
			}
			d->PluginPathMap.insert(id, info.absoluteFilePath());
			if (!d->PriorityMap.contains(id)) {
				d->PriorityMap.insert(id, 0);
			}
			yMessage << "Plugin with meta-id" << id << "finded";
			if (!jsonConfig.contains("Dependencies")) {
				continue;
			}
			QStringList dependIndex = jsonConfig.keys("Dependencies");
			for (QString de : dependIndex) {
				QString dependID = jsonConfig.getString("Dependencies." + de);
				if (!d->PriorityMap.contains(dependID)) {
					d->PriorityMap.insert(dependID, 1);
				}
				else {
					d->PriorityMap[dependID] += 1;
				}
			}
		}
		// TODO: check whether all dependencies are satisfied. need to add a dependency list.
		// if not satisfied, raise exception and stop loading.
		
		// sort by priority, save path to PluginsPaths
		yMessage << "Determining loading order based on priority";
		d->PriorityPlugins = d->PriorityMap.keys();
		std::sort(d->PriorityPlugins.begin(), d->PriorityPlugins.end(), [this](const QString& a, const QString& b) {
			return d->PriorityMap[a] > d->PriorityMap[b];
			});
		for (int i = 0; i < d->PriorityPlugins.length(); i++) {
			yMessage << "Load order:" << d->PriorityPlugins[i] << "[" << i << "]";
		}
		for (QString key : d->PriorityPlugins) {
			if (d->PluginPathMap.contains(key)) {
				QString path = d->PluginPathMap.value(key);
				QLibrary* hLibrary = new QLibrary(path);
				if (hLibrary->load() == false) {
					yError << "Failed when load plugin" << key << ", cannot load plugin file into memory!";
					return;
				}
				__VisindigoPluginMain PluginDllMain = (__VisindigoPluginMain)hLibrary->resolve(Visindigo_PluginMain_Function_Name);
				if (PluginDllMain == nullptr) {
					yError << "Failed when load plugin" << key << ", cannot find entry point VisindigoPluginMain!";
					hLibrary->unload();
					delete hLibrary;
					return;
				}
				Plugin* plugin = nullptr;
				try {
					plugin = PluginDllMain();
				}
				catch (...) {
					yError << "Exception occured when load plugin" << key << ", exception has been catched, but may have other impace. RESTART is RECOMMENDED! ";
				}
				if (plugin == nullptr) {
					yError << "Failed when init plugin" << key << ", cannot create EditorPlugin Instance!";
					hLibrary->unload();
					return;
				}
				QString pluginID = plugin->getPluginID();
				if (pluginID != key) {
					yError << "Failed when init plugin" << key << ", the ID from plugin instance is different from the ID from meta file.";
					delete plugin;
					hLibrary->unload();
					continue;
				}
				d->Plugins.append(plugin);
				d->Dlls.insert(plugin->getPluginID(), hLibrary);
				d->PluginIDMap.insert(plugin->getPluginID(), plugin);
				plugin->d->PluginFolder = path;
				emit pluginLoaded(plugin);
				ySuccess << "Plugin" << key << "create instance successfully. Will be enable later";
			}
		}
	}

	void PluginManager::testAllPlugin() {
		for (int i = 0; i < d->Plugins.size(); i++) {
			Plugin* plugin = d->Plugins[i];
			if (plugin->isTestEnable()) {
				yMessage << "Testing plugin" << plugin->getPluginName();
				plugin->onTest();
			}
		}
	}

	void PluginManager::applicationInitAllPlugin() {
		for (int i = 0; i < d->Plugins.size(); i++) {
			Plugin* plugin = d->Plugins[i];
			yMessage << plugin->getPluginName() << " is handling application init";
			plugin->onApplicationInit();
		}
	}

	/*!
		\since Visindigo 0.13.0
		禁用所有已启用的插件。请在调用enableAllPlugin()之后调用此函数。
	*/
	void PluginManager::disableAllPlugin() {
		// disable in reverse order
		for (int i = d->PriorityPlugins.size() - 1; i >= 0; i--) {
			Plugin* plugin = d->PluginIDMap[d->PriorityPlugins[i]];
			if (isPluginEnable(plugin)){
				try {
					yMessage << "Trying to disable plugin" << plugin->getPluginName();
					plugin->onPluginDisbale();
				}
				catch (...) {
					yError << "Failed when disable plugin" << plugin->getPluginName() << ", disable function may not have completed properly. RESTART is RECOMMENDED!";
					continue;
				}
				ySuccess << "Plugin" << plugin->getPluginName() << "disabled";
				d->EnabledPlugins.removeAll(plugin);
			}
		}
	}

	qint32 PluginManager::getLoadedPluginCount() const {
		return d->Plugins.size();
	}

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

	bool PluginManager::isPluginEnable(Plugin* plugin) const {
		return d->EnabledPlugins.contains(plugin);
	}
	/*!
		\since Visindigo 0.13.0
		启用所有已加载但未启用的插件。请在调用programLoadPlugin()之后调用此函数。
	*/
	void PluginManager::enableAllPlugin() {
		for (int i = 0; i < d->PriorityPlugins.size(); i++) {
			Plugin* plugin = d->PluginIDMap[d->PriorityPlugins[i]];
			if (!isPluginEnable(plugin)) {
				try {
					yMessage << "Trying to enable plugin" << plugin->getPluginName();
					plugin->onPluginEnable();
				}
				catch (...) {
					yError << "Failed when enable plugin" << plugin->getPluginName() << ", disable function will be called. RESTART is RECOMMENDED!";
					plugin->onPluginDisbale();
					continue;
				}
				ySuccess << "Plugin" << plugin->getPluginName() << "enabled";
				emit pluginEnabled(plugin);
				d->EnabledPlugins.append(plugin);
			}
		}
	}
}