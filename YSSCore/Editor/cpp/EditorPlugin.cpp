#include <QtCore/qstring.h>
#include "../EditorPlugin.h"
#include "../LangServerManager.h"
#include "../FileServerManager.h"
#include "../../General/TranslationHost.h"
namespace YSSCore::Editor {
	class EditorPluginPrivate {
		friend class EditorPlugin;
	protected:
		YSSCore::General::Version ABIVersion = YSSCore::General::Version(0, 0, 0);
		YSSCore::General::Version PluginVersion = YSSCore::General::Version(0, 0, 0);
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QString PluginFolder;
	};

	/*! 
		\class YSSCore::Editor::EditorPlugin
		\inmodule YSSCore
		\brief 此类为Yayin Story Studio 提供插件基类。
		\since YSSCore 0.13.0

		EditorPlugin是所有插件的基类，所有插件都需要继承此类。

		要开发Yayin Story Studio插件，您就必须实现此类，然后在您的项目中声明YSSPluginDllMain函数。这个
		函数只做一件事情：新建您的插件类，并且返回它。
		\code
		    extern "C" YSSCore::Editor::EditorPlugin* YSSPluginDllMain() {
		        return new YourPluginClass();
		    }
		\endcode
		\warning 请注意，YSSPluginDllMain函数必须是extern "C"的，否则Yayin Story Studio将无法找到它。

		稍后，Yayin Story Studio将会调用您的插件的onPluginEnable()函数来启用插件。您应该在这个函数中
		进行一些初始化工作，比如注册语言服务器、文件服务器、翻译器等。
		
		您不应该在您的插件的构造函数中进行除了基本信息设置之外的任何其他工作，尤其是尝试和YSS或其他插件交互。
		因为在构造插件的过程中，YSS尚未准备好全部的程序功能，您极有可能遭遇nullptr。

		我们以YSS自带的ASEDevTool插件为例，向您展示插件的各个函数中应该做什么：
		\code
			Plugin_ASEDevTool::Plugin_ASEDevTool() {
				setPluginVersion(YSSCore::General::Version(0, 1, 0));
				setPluginID("ASEDevTool");
				setPluginName("ASE Development Tool");
				setPluginAuthor({ "Gra_dus" });
			}
			void Plugin_ASEDevTool::onPluginEnable() {
				registerTranslator(new ASEDevTranslator());
				registerLangServer(new AStoryLanguageServer());
				registerFileServer(new AStoryFileServer());
				registerFileServer(new ASRuleFileServer());
			}

			void Plugin_ASEDevTool::onPluginDisbale() {
			}
		\endcode

		要使Yayin Story Studio能够找到您的插件，您还需要至少三步操作：

		1. 在应用程序resources/plugins/目录下创建一个文件夹，命名任选，最好为您的插件ID。

		2. 确保您编译得到的动态链接库文件的后缀名为.ysp（这是全平台统一的），并且放在您的插件文件夹中。

		3. 在您的插件文件夹中新建一个与您插件文件同名的json文件，例如“yourplugin.ysp.json"，并做如下编辑：
		\badcode
			{
				"ID": "YourPluginID",
				"Depend": [
					“PluginID1",
					“PluginID2"
				]
			}
		\endcode

		Yayin Story Studio在启动时会首先搜索plugins文件夹下的所有ysp文件，并且依赖这个与其同名的JSON文件
		分析插件的依赖信息并决定加载顺序。json文件中的ID必须和代码中实际设置的ID一致，否则Yayin Story Studio将无法
		找到您的插件。Depend字段是一个字符串数组，表示您的插件依赖的其他插件的ID。Yayin Story Studio会在加载
		插件时，先加载这些依赖的插件，然后再加载您的插件。
		
		请注意，Yayin Story Studio无法处理循环依赖。当两个插件相互依赖时，将具有相同的优先等级，YSS无法确定其加载顺序。

		如果您的插件不依赖其他插件，可以将Depend字段省略。
	*/

	/*!
		\a abiVersion 插件的ABI版本
		\a parent 插件的父对象
		\since YSSCore 0.13.0

		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时YSS对应的ABI版本。
		在派生此类时，不需要实际传递任何参数。
		\warning 请注意，abiVersion并不是插件的版本号，也不是YSS的程序版本。它是YSS的二进制兼容版本。
	*/
	EditorPlugin::EditorPlugin(YSSCore::General::Version abiVersion, QObject* parent) : QObject(parent) {
		d = new EditorPluginPrivate();
		d->ABIVersion = abiVersion;
	}
	/*!
		\since YSSCore 0.13.0
		析构函数
	*/
	EditorPlugin::~EditorPlugin() {
		delete d;
	}
	/*!
		\since YSSCore 0.13.0
		return 插件的ID
	*/
	QString EditorPlugin::getPluginID() const {
		return d->PluginID;
	}
	/*!
		\since YSSCore 0.13.0
		return 插件的名称
	*/
	QString EditorPlugin::getPluginName() const {
		return d->PluginName;
	}
	/*!
		\since YSSCore 0.13.0
		return 插件的作者，这是一个列表。
	*/
	QStringList EditorPlugin::getPluginAuthor() const {
		return d->PluginAuthor;
	}
	/*!
		\since YSSCore 0.13.0
		return 插件的根目录
	*/
	QString EditorPlugin::getPluginFolder() const {
		return d->PluginFolder;
	}
	/*!
		\since YSSCore 0.13.0
		\a id 插件的ID
		设置插件的ID
	*/
	void EditorPlugin::setPluginID(const QString& id) {
		d->PluginID = id;
	}
	/*!
		\since YSSCore 0.13.0
		\a name 插件的名称
		设置插件的名称
	*/
	void EditorPlugin::setPluginName(const QString& name) {
		d->PluginName = name;
	}
	/*!
		\since YSSCore 0.13.0
		\a author 插件的作者
		设置插件的作者
	*/
	void EditorPlugin::setPluginAuthor(const QStringList& author) {
		d->PluginAuthor = author;
	}
	/*!
		\since YSSCore 0.13.0
		\a folder 插件的根目录
		设置插件的根目录
		\warning 此函数理应被弃用，因为插件的根目录应由YSS强制指定。
	*/
	void EditorPlugin::setPluginFolder(const QString& folder) {
		d->PluginFolder = folder;
	}
	/*!
		\since YSSCore 0.13.0
		\a version 插件的版本号
		设置插件的版本号
	*/
	void EditorPlugin::setPluginVersion(const YSSCore::General::Version& version) {
		d->PluginVersion = version;
	}
	/*!
		\since YSSCore 0.13.0
		\a server 语言服务器
		注册语言服务器。
		\note 这里的语言服务器指的是代码语言高亮、着色与分析，而非自然语言翻译器。
	*/
	void EditorPlugin::registerLangServer(LangServer* server) {
		LangServerManager::getInstance()->addLangServer(server);
	}
	/*!
		\since YSSCore 0.13.0
		\a server 调试服务器
		注册调试服务器。
	*/
	void EditorPlugin::registerDebugServer(DebugServer* server) {
		//TODO;
	}
	/*!
		\since YSSCore 0.13.0
		\a server 文件服务器
		注册文件服务器。
	*/
	void EditorPlugin::registerFileServer(FileServer* server) {
		FileServerManager::getInstance()->registerFileServer(server);
	}

	/*!
		\since YSSCore 0.13.0
		\a translator 翻译器
		注册翻译器。
	*/
	void EditorPlugin::registerTranslator(YSSCore::General::Translator* translator) {
		YSSCore::General::TranslationHost::getInstance()->active(translator);
	}
	/*!
		\since YSSCore 0.13.0
		\a provider 项目模板提供者
		注册项目模板提供者。
	*/
	void EditorPlugin::registerProjectTemplateProvider(ProjectTemplateProvider* provider) {
		//TODO;
	}
}