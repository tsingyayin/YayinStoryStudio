#include <QtCore/qstring.h>
#include <General/TranslationHost.h>
#include "Editor/ColorThemeProvider.h"
#include "Editor/DebugServer.h"
#include "Editor/DebugServerManager.h"
#include "Editor/EditorPlugin.h"
#include "Editor/FileServer.h"
#include "Editor/FileServerManager.h"
#include "Editor/FileTemplateManager.h"
#include "Editor/FileTemplateProvider.h"
#include "Editor/LangServer.h"
#include "Editor/LangServerManager.h"
#include "Editor/private/EditorPlugin_p.h"
#include "Editor/ProjectTemplateManager.h"
#include "Editor/ProjectTemplateProvider.h"
#include "General/YSSProject.h"
namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::EditorPlugin
		\inmodule YSSCore
		\brief 此类为Yayin Story Studio提供插件基类.
		\since YSSCore 0.13.0

		EditorPlugin是所有YSS编辑器插件的基类。除非只需要提供标准Visindigo插件功能，
		否则您应该从此类派生来开发您的插件。

		这类的用法和Visindigo::General::Plugin类似，但提供了更多与YSS编辑器交互的功能。
	*/

	/*!
		\since YSS 0.13.0
		\a id 插件的ID
		\a apiVersion 插件的API版本
		\a abiVersion 插件的ABI版本
		\a parent 插件的父对象

		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时YSS对应的ABI版本。
		在派生此类时，不需要实际传递任何参数。
		\warning 请注意，abiVersion并不是插件的版本号，也不是YSS的程序版本。它是YSS的二进制兼容版本。
	*/
	EditorPlugin::EditorPlugin(const QString& id, Visindigo::General::Version apiVersion, Visindigo::General::Version abiVersion, QObject* parent) :
		Visindigo::General::Plugin(id, apiVersion, abiVersion, YSSPluginTypeID, parent) {
		d = new YSSCore::__Private__::EditorPluginPrivate();
	}
	/*!
		\since YSS 0.13.0
		析构函数
	*/
	EditorPlugin::~EditorPlugin() {
		delete d;
	}

	/*!
		\fn YSSCore::Editor::EditorPlugin::onProjectOpen(YSSCore::General::YSSProject* project)
		\since YSS 0.13.0
		\a project 打开的项目对象

		打开项目时调用此虚函数。参数project是被打开的项目对象。
	*/

	/*!
		\fn YSSCore::Editor::EditorPlugin::onProjectClose(YSSCore::General::YSSProject* project)
		\since YSS 0.13.0
		\a project 关闭的项目对象

		关闭项目时调用此虚函数。参数project是被关闭的项目对象。
	*/

	/*!
		\fn bool YSSCore::Editor::EditorPlugin::onProjectAboutToClose(YSSCore::General::YSSProject* project)
		\since YSSCore 0.16.0
		\a project 即将关闭的项目对象

		return 同意关闭项目时返回 true，否则返回 false。
		当项目即将被关闭时调用此函数。参数project是即将被关闭的项目对象。
		YSS会在关闭项目之前调用此函数，询问插件是否同意关闭项目。如果所有插件都同意，则项目正常关闭；
		如果有任何一个插件不同意，则取消关闭项目的操作。因此，如果您的插件需要在项目关闭前进行一些检查，
		或者需要提示用户保存未保存的更改，您可以在这个函数中实现相关逻辑，并根据情况返回true或false。

		默认实现返回true，表示同意关闭项目。您可以重写此函数来实现自定义的关闭前逻辑。

		请注意，根据YSSCore的实现，只要有插件不同意关闭项目，则后续的onProjectAboutToClose都不会再继续调用。
		因此，不应当在这个函数中执行资源回收或其他影响插件状态的操作，只判断当前是否可以关闭项目即可。
	*/

	/*!
		\since YSS 0.13.0
		\a server 语言服务器

		注册语言服务器。此函数还会自动从插件配置中读取该语言服务器的当前主题，并设置给语言服务器的颜色主题提供者。
	*/
	void EditorPlugin::registerLangServer(LangServer* server) {
		registerPluginModule(server);
		QString currentThemeName = getPluginConfig()->getString("_yss_auto_.LangServer." + server->getModuleID() + ".CurrentTheme");
		if (not currentThemeName.isEmpty()) {
			server->getColorThemeProvider()->setCurrentTheme(currentThemeName);
		}
		connect(server->getColorThemeProvider(), &ColorThemeProvider::currentThemeChanged, this, [this, server](const QString& newThemeName) {
			getPluginConfig()->setString("_yss_auto_.LangServer." + server->getModuleID() + ".CurrentTheme", newThemeName);
			savePluginConfig();
			});
		LangServerManager::getInstance()->addLangServer(server);
	}
	/*!
		\since YSS 0.13.0
		\a server 调试服务器

		注册调试服务器。
	*/
	void EditorPlugin::registerDebugServer(DebugServer* server) {
		registerPluginModule(server);
		DebugServerManager::getInstance()->addDebugServer(server);
	}
	/*!
		\since YSS 0.13.0
		\a server 文件服务器

		注册文件服务器。
	*/
	void EditorPlugin::registerFileServer(FileServer* server) {
		registerPluginModule(server);
		FileServerManager::getInstance()->registerFileServer(server);
	}

	/*!
		\since YSS 0.13.0
		\a provider 项目模板提供者

		注册项目模板提供者。
	*/
	void EditorPlugin::registerProjectTemplateProvider(ProjectTemplateProvider* provider) {
		registerPluginModule(provider);
		YSSCore::Editor::ProjectTemplateManager::getInstance()->addProvider(provider);
	}

	/*!
		\since YSS 0.13.0
		\a provider 文件模板提供者

		注册文件模板提供者。
	*/
	void EditorPlugin::registerFileTemplateProvider(FileTemplateProvider* provider) {
		registerPluginModule(provider);
		YSSCore::Editor::FileTemplateManager::getInstance()->addProvider(provider);
	}

	/*!
		\since YSS 0.15.0
		return 当前打开的项目。

		实际上，这个函数就是YSSProject::getCurrentProject的本类便捷写法。
	*/
	YSSCore::General::YSSProject* EditorPlugin::getCurrentProject() {
		return YSSCore::General::YSSProject::getCurrentProject();
	}
}