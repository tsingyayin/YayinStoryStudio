#include "../EditorPluginManager.h"
#include "../EditorPlugin.h"
#include "../../Utility/JsonConfig.h"
#include <QtCore/qstring.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qmap.h>
#include "../../Utility/FileUtility.h"
#include "../../General/Log.h"
#include "../private/EditorPlugin_p.h"

namespace Visindigo::Editor {
	class EditorPluginManagerPrivate {
		friend class EditorPluginManager;
	protected:
		QList<EditorPlugin*> Plugins;
		QMap<EditorPlugin*, bool> PluginEnable;
		QMap<QString, EditorPlugin*> PluginIDMap;
		QMap<QString, quint32> PriorityMap;
		QList<QString> PriorityPlugins;
		QMap<QString, QString> PluginPathMap;
		QMap<QString, QLibrary*> Dlls;
		static QFileInfoList recursionGetAllDll(const QString& path) {
			QDir dir(path);
			QStringList filters;
			filters << "*.ysp";
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
	/*!
		\class Visindigo::Editor::EditorPluginManager
		\inmodule Visindigo
		\brief 此类为YayinStoryStudio提供插件管理器。
		\since Visindigo 0.10.0

		EditorPluginManager负责加载和管理插件。
	*/

	/*!
		\since Visindigo 0.10.0
		\a parent 为父对象。
		构造EditorPluginManager对象。
	*/
	EditorPluginManager::EditorPluginManager(QObject* parent) : QObject(parent) {
		d = new EditorPluginManagerPrivate();
		ySuccessF << "Success!";
	}
	EditorPluginManager::~EditorPluginManager() {
		for (int i = 0; i < d->Plugins.size(); i++) {
			delete d->Plugins[i];
		}
		d->Plugins.clear();
		delete d;
	}
	void EditorPluginManager::programLoadPlugin() {
		yNotice << "Scanning plugins in ./resouces/plugins";
		QFileInfoList Plugins = EditorPluginManagerPrivate::recursionGetAllDll("./resource/plugins");
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
					yError << "Failed when init plugin" << key << ", cannot load into memory!";
					return;
				}
				__YSSPluginDllMain PluginDllMain = (__YSSPluginDllMain)hLibrary->resolve("YSSPluginDllMain");
				if (PluginDllMain == nullptr) {
					yError << "Failed when init plugin" << key << ", cannot find entry point YSSPluginDllMain!";
					hLibrary->unload();
					delete hLibrary;
					return;
				}
				EditorPlugin* plugin = nullptr;
				try {
					plugin = PluginDllMain();
				}
				catch (...) {
					yError << "Exception occured when init plugin" << key << ", exception has been catched, but may have other impace. RESTART is RECOMMENDED! ";
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
				ySuccess << "Plugin" << key << "create instance successfully. Will be enable later";
			}
		}
	}
	bool EditorPluginManager::isPluginEnable(const QString& id) const {
		if (d->PluginIDMap.contains(id)) {
			return d->PluginEnable.value(d->PluginIDMap.value(id));
		}
		return false;
	}
	void EditorPluginManager::loadPlugin() {
		for (int i = 0; i < d->PriorityPlugins.size(); i++) {
			EditorPlugin* plugin = d->PluginIDMap[d->PriorityPlugins[i]];
			if (d->PluginEnable.value(plugin) == false) {
				try {
					yMessage << "Trying to enable plugin" << plugin->getPluginName();
					plugin->onPluginEnable();
				}
				catch (...) {
					yError << "Failed when enable plugin" << plugin->getPluginName() << ", disable function will be called. RESTART is RECOMMENDED!";
					plugin->onPluginDisbale();
					d->PluginEnable.insert(plugin, false);
					continue;
				}
				ySuccess << "Plugin" << plugin->getPluginName() << "enabled";
				d->PluginEnable.insert(plugin, true);
			}
		}
	}
}