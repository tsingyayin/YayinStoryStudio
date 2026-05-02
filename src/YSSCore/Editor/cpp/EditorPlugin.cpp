#include <QtCore/qstring.h>
#include <General/TranslationHost.h.>
#include "../DebugServerManager.h"
#include "../private/EditorPlugin_p.h"
#include "../EditorPlugin.h"
#include "../LangServerManager.h"
#include "../FileServerManager.h"
#include "../ProjectTemplateManager.h"
#include "../FileTemplateManager.h"
#include "../LangServer.h"
#include "../DebugServer.h"
#include "../FileServer.h"
#include "../ProjectTemplateProvider.h"
#include "../FileTemplateProvider.h"

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::EditorPlugin
		\inmodule YSSCore
		\brief 此类为Yayin Story Studio提供插件基类。
		\since YSSCore 0.13.0

		EditorPlugin是所有YSS编辑器插件的基类。除非只需要提供标准Visindigo插件功能，
		否则您应该从此类派生来开发您的插件。

		这类的用法和Visindigo::General::Plugin类似，但提供了更多与YSS编辑器交互的功能。
	*/

	/*!
		\a abiVersion 插件的ABI版本
		\a parent 插件的父对象
		\since Visindigo 0.13.0

		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时YSS对应的ABI版本。
		在派生此类时，不需要实际传递任何参数。
		\warning 请注意，abiVersion并不是插件的版本号，也不是YSS的程序版本。它是YSS的二进制兼容版本。
	*/
	EditorPlugin::EditorPlugin(Visindigo::General::Version apiVersion, Visindigo::General::Version abiVersion, QObject* parent) :
		Visindigo::General::Plugin(apiVersion, abiVersion, YSSPluginTypeID, parent) {
		d = new YSSCore::__Private__::EditorPluginPrivate();
	}
	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	EditorPlugin::~EditorPlugin() {
		delete d;
	}

	/*!
		\fn YSSCore::Editor::EditorPlugin::onPluginEnable()
		\since Visindigo 0.13.0
		启用插件时调用此函数。大部分涉及到Visindigo内部资源调用的初始化都应该在此函数中进行。

		如果在构造函数中初始化，则有可能因为Visindigo尚未完全初始化而导致崩溃。此外，此函数
		还会被Visindigo捕获异常并输出到日志中。
	*/

	/*!
		\fn YSSCore::Editor::EditorPlugin::onPluginDisable()
		\since Visindigo 0.13.0
		禁用插件时调用此函数。大部分涉及到Visindigo内部资源调用的清理都应该在此函数中进行。
		如果在析构函数中清理，则有可能因为Visindigo已经开始析构而导致崩溃。此外，此函数
		还会被Visindigo捕获异常并输出到日志中。
	*/

	void EditorPlugin::registerLangServer(LangServer* server) {
		registerPluginModule(server);
		LangServerManager::getInstance()->addLangServer(server);
	}
	/*!
		\since Visindigo 0.13.0
		\a server 调试服务器
		注册调试服务器。
	*/
	void EditorPlugin::registerDebugServer(DebugServer* server) {
		registerPluginModule(server);
		DebugServerManager::getInstance()->addDebugServer(server);
	}
	/*!
		\since Visindigo 0.13.0
		\a server 文件服务器
		注册文件服务器。
	*/
	void EditorPlugin::registerFileServer(FileServer* server) {
		registerPluginModule(server);
		FileServerManager::getInstance()->registerFileServer(server);
	}

	/*!
		\since Visindigo 0.13.0
		\a provider 项目模板提供者
		注册项目模板提供者。
	*/
	void EditorPlugin::registerProjectTemplateProvider(ProjectTemplateProvider* provider) {
		registerPluginModule(provider);
		YSSCore::Editor::ProjectTemplateManager::getInstance()->addProvider(provider);
	}

	/*!
		\since Visindigo 0.13.0
		\a provider 文件模板提供者
		注册文件模板提供者。
	*/
	void EditorPlugin::registerFileTemplateProvider(FileTemplateProvider* provider) {
		registerPluginModule(provider);
		YSSCore::Editor::FileTemplateManager::getInstance()->addProvider(provider);
	}

}