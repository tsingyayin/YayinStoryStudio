#include <General/Version.h>
#include "Editor/YSSCoreBuiltinPlugin.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	YSSCoreBuiltinPlugin* YSSCoreBuiltinPlugin::Instance = nullptr;

	/*!
		\class YSSCore::Editor::YSSCoreBuiltinPlugin
		\inmodule YSSCore
		\brief YSSCore的内置功能插件.
		\since YSS 0.17.0

		YSS内置了部分需要以插件形式运作的功能，这些功能需要有一个插件作为宿主，
		YSSCoreBuiltinPlugin就是这样一个内置插件。例如，用于打开不受任何FileServer
		支持的文件的内置文件服务，就以本插件作为其宿主插件。

		此插件不由YSSCore自行创建或启用。宿主应用程序需要在启动之前创建它，并通过
		VIApplication::addDependencyPlugin()将其登记为依赖插件。
	*/

	/*!
		\since YSS 0.17.0
		构造YSSCoreBuiltinPlugin对象。
	*/
	YSSCoreBuiltinPlugin::YSSCoreBuiltinPlugin() :EditorPlugin("cn.yxgeneral.yss.builtin.core") {
		Instance = this;
		setPluginVersion(Visindigo::General::Version::getAPIVersion());
		setPluginName("YSSCore Builtin");
		setPluginAuthor({ "Tsing Yayin" });
	}

	/*!
		\since YSS 0.17.0
		析构YSSCoreBuiltinPlugin对象。
	*/
	YSSCoreBuiltinPlugin::~YSSCoreBuiltinPlugin() {
	}

	/*!
		\since YSS 0.17.0
		return YSSCoreBuiltinPlugin的单例对象。

		若单例尚未创建，则会在此函数中创建它。宿主应用程序应当通过此函数获取实例，
		以确保整个程序内使用的是同一个YSSCoreBuiltinPlugin对象。
	*/
	YSSCoreBuiltinPlugin* YSSCoreBuiltinPlugin::getInstance() {
		if (Instance == nullptr) {
			Instance = new YSSCoreBuiltinPlugin();
		}
		return Instance;
	}

	/*!
		\since YSS 0.17.0
		启用插件。
	*/
	void YSSCoreBuiltinPlugin::onPluginEnable() {
		yInfo << "YSSCoreBuiltin plugin enabled.";
	}
}
