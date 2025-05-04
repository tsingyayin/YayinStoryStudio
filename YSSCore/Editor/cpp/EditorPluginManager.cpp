#include "../EditorPluginManager.h"
#include "../EditorPlugin.h"
#include <Windows.h>
#include "../../Utility/JsonConfig.h"

namespace YSSCore::Editor {
	class EditorPluginManagerPrivate {
		friend class EditorPluginManager;
	protected:
		QList<EditorPlugin*> Plugins;
		QMap<EditorPlugin*, bool> PluginEnable;
		QMap<QString, EditorPlugin*> PluginIDMap;
		QMap<QString, quint32> PriorityMap;
		QList<QString> PriorityPlugins;
		QMap<QString, QString> PluginPathMap;
		QMap<QString, HMODULE> Dlls;
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
	EditorPluginManager::EditorPluginManager(QObject* parent) : QObject(parent) {
		p = new EditorPluginManagerPrivate();
	}
	EditorPluginManager::~EditorPluginManager() {
		for (int i = 0; i < p->Plugins.size(); i++) {
			delete p->Plugins[i];
		}
		p->Plugins.clear();
		delete p;
	}
	void EditorPluginManager::programLoadPlugin() {
		QFileInfoList Plugins = EditorPluginManagerPrivate::recursionGetAllDll("./resource/plugins");
		for (QFileInfo info : Plugins) {
			QString path = info.absoluteFilePath() + ".json";
			QFile file(path);
			if (!file.open(QIODevice::ReadOnly)) {
				qDebug() << "EditorPluginManager: open plugin meta file failed!";
				continue;
			}
			QTextStream in(&file);
			in.setEncoding(QStringConverter::Utf8);
			QString jsonStr = in.readAll();
			file.close();
			Utility::JsonConfig jsonConfig(jsonStr);
			QString id = jsonConfig.getString("ID");
			p->PluginPathMap.insert(id, info.absoluteFilePath());
			if (!p->PriorityMap.contains(id)) {
				p->PriorityMap.insert(id, 0);
			}
			qDebug() << "EditorPluginManager: plugin id:" << id;
			if (!jsonConfig.contains("Dependencies")) {
				continue;
			}
			QStringList dependIndex = jsonConfig.keys("Dependencies");
			for (QString d : dependIndex) {
				QString dependID = jsonConfig.getString("Dependencies." + d);
				if (!p->PriorityMap.contains(dependID)) {
					p->PriorityMap.insert(dependID, 1);
				}
				else {
					p->PriorityMap[dependID] += 1;
				}
			}
		}
		// sort by priority, save path to PluginsPaths
		p->PriorityPlugins = p->PriorityMap.keys();
		std::sort(p->PriorityPlugins.begin(), p->PriorityPlugins.end(), [this](const QString& a, const QString& b) {
			return p->PriorityMap[a] > p->PriorityMap[b];
			});
		for (QString key : p->PriorityPlugins) {
			if (p->PluginPathMap.contains(key)) {
				QString path = p->PluginPathMap.value(key);
				HMODULE hModule = LoadLibraryA(path.toStdString().c_str());
				if (hModule == nullptr) {
					qDebug() << "EditorPluginManager: load dll failed!";
					return;
				}
				__YSSPluginDllMain PluginDllMain = (__YSSPluginDllMain)(GetProcAddress(hModule, "YSSPluginDllMain"));
				if (PluginDllMain == nullptr) {
					qDebug() << "EditorPluginManager: get dll entry point failed!";
					FreeLibrary(hModule);
					return;
				}
				EditorPlugin* plugin = PluginDllMain();
				if (plugin == nullptr) {
					qDebug() << "EditorPluginManager: create EditorPlugin failed!";
					FreeLibrary(hModule);
					return;
				}
				p->Plugins.append(plugin);
				p->Dlls.insert(plugin->getPluginID(), hModule);
				p->PluginIDMap.insert(plugin->getPluginID(), plugin);
				plugin->setPluginFolder(path);
				qDebug() << "EditorPluginManager: load plugin:" << plugin->getPluginID();
			}
		}
	}
	bool EditorPluginManager::isPluginEnable(const QString& id) const {
		if (p->PluginIDMap.contains(id)) {
			return p->PluginEnable.value(p->PluginIDMap.value(id));
		}
		return false;
	}
	void EditorPluginManager::loadPlugin() {
		for (int i = 0; i < p->PriorityPlugins.size(); i++) {
			EditorPlugin* plugin = p->PluginIDMap[p->PriorityPlugins[i]];
			if (p->PluginEnable.value(plugin) == false) {
				try {
					plugin->onPluginEnable();
				}
				catch (...) {
					qDebug() << "EditorPluginManager: plugin enable failed!";
					plugin->onPluginDisbale();
					p->PluginEnable.insert(plugin, false);
					continue;
				}
				p->PluginEnable.insert(plugin, true);
			}
		}
	}
}