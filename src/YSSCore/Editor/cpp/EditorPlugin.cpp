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
		\since YSS 0.13.0

		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时YSS对应的ABI版本。
		在派生此类时，不需要实际传递任何参数。
		\warning 请注意，abiVersion并不是插件的版本号，也不是YSS的程序版本。它是YSS的二进制兼容版本。
	*/
	EditorPlugin::EditorPlugin(Visindigo::General::Version apiVersion, Visindigo::General::Version abiVersion, QObject* parent) :
		Visindigo::General::Plugin(apiVersion, abiVersion, YSSPluginTypeID, parent) {
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
		打开项目时调用此函数。参数project是被打开的项目对象。
	*/

	/*!
		\fn YSSCore::Editor::EditorPlugin::onProjectClose(YSSCore::General::YSSProject* project)
		\since YSS 0.13.0
		关闭项目时调用此函数。参数project是被关闭的项目对象。
	*/

	/*!
		\fn YSSCore::Editor::EditorPlugin::onToolWidgetRequested(const QString& widgetID)
		\since YSSCore 0.15.0
		YSS需要请求显示工具窗口内容时调用此函数。参数widgetID是被请求的工具窗口ID。
		如果插件提供了对应ID的工具窗口内容，则返回一个QWidget指针；否则返回nullptr。
		
		这个窗口一旦创建，所有权归YSS前台所有，插件不需要也不应该管理它的生命周期。
		YSS会确保每次请求同一个工具窗口ID时，上一次请求的已经被正确销毁了，因此插件不需要担心重复创建工具窗口的问题。

		这个窗口不应该中途移除Qt::WA_DeleteOnClose属性，YSS使用此属性来管理工具窗口的生命周期。
	*/
	QWidget* EditorPlugin::onToolWidgetRequested(const QString& widgetID) {
		return nullptr;
	}


	void EditorPlugin::registerLangServer(LangServer* server) {
		registerPluginModule(server);
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
		\a widgetID 工具窗口ID
		\a widgetName 工具窗口名称
		注册工具窗口。工具窗口是YSS编辑器中可以停靠的窗口，插件可以通过onToolWidgetRequested()提供工具窗口的内容。

		工具窗口的ID必须全程序唯一，一般建议使用反向域命名法。\a widgetName是支持VI18N的字符串，用于直接显示在界面上。
	*/
	void EditorPlugin::registerToolWidget(const QStringList& widgetID, const QString& widgetName) {
		// TODO.
	}

}