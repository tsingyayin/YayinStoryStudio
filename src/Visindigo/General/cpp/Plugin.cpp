#include <QtCore/qstring.h>
#include "General/TranslationHost.h"
#include "General/private/Plugin_p.h"
#include "General/Plugin.h"
#include "General/PluginModule.h"
#include "General/Version.h"
#include "General/Logger.h"
#include "Widgets/ThemeManager.h"
#include "Utility/FileUtility.h"

namespace Visindigo::__Private__ {
	void PluginPrivate::initializePluginFolder(const QDir& baseDir) {
		PluginFolder = QDir(baseDir.filePath(PluginID));
		if (!PluginFolder.exists()) {
			PluginFolder.mkpath(".");
		}
	}

	bool PluginPrivate::enablePlugin() {
		try {
			q->onPluginEnable();
			for (auto module : Modules) {
				module->onModuleEnable();
			}
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool PluginPrivate::disablePlugin() {
		try {
			for (int i = Modules.size() - 1; i >= 0; --i) {
				auto module = Modules.at(i);
				module->onModuleDisable();
			}
			q->onPluginDisbale();
		}
		catch (...) {
			return false;
		}
		return true;
	}
}
namespace Visindigo::General {
	/*!
		\class Visindigo::General::Plugin
		\inheaderfile General/Plugin.h
		\inmodule Visindigo
		\ingroup VPlugin
		\brief 此类为Visindigo提供插件基类。
		\since Visindigo 0.13.0

		要开发Visindigo插件或应用程序，您就必须实现此类的一个派生。即Visindigo将基于其实现的插件
		与基于其实现的应用程序按等同逻辑处理。唯一区别在于它们的加载与卸载顺序，这在Visindigo::General::VIApplication中
		有详细说明。

		\section1 主要函数说明

		Plugin类最重要的函数有五个，是用户必须实现的函数，即构造函数，onPluginEnable()，onPluginDisbale()，onApplicationInit()和onTest()。
		
		无论是开发应用程序还是插件，这五个函数都遵循相同的逻辑与原则：
		\list
		\li 构造函数：只用于设置本插件或本应用程序的基础信息，如setPluginID()，setPluginName()，setPluginAuthor()等。除此之外
		不应执行任何其他操作，尤其是与Visindigo或其他插件交互的操作，因为在构造函数执行时，Visindigo尚未准备好全部功能，您极有可能遭遇nullptr。
		\li onPluginEnable()：当插件或应用程序被启用时调用。在此函数中，您应进行插件或应用程序的初始化工作，大部分构造操作应该在此进行
		\li onApplicationInit()：当整个应用程序所有的插件都被启用后，它会被按一定顺序调用。您可以在此函数中执行一些需要
		所有插件都已启用后才能进行的操作
		\li onTest()：如果您的插件或应用程序被设置为启用测试功能（通过调用setTestEnable()函数），则在所有插件的onApplicationInit()函数调用完毕后，
		它会被按一定顺序调用。您可以在此函数中执行一些测试代码
		\li onPluginDisbale()：当插件或应用程序被禁用时调用。在此函数中，您应进行插件或应用程序的清理工作，大部分析构造操作应该在此进行。
		\li 析构函数：不推荐实现这个函数，保留为编译器默认值即可。如果您确实需要实现它，请确保在析构函数中不与Visindigo或其他插件交互。
		\endlist

		\section1 作为应用程序主插件（主应用程序包）

		需要事先说明的是，如果您不习惯用“插件”称呼您的应用程序，您也可以使用我们在Package.h中提供的别名：
		\list
		\li Visindigo::General::Package （等同于Visindigo::General::Plugin）
		\li Visindigo::General::PackageModule （等同于Visindigo::General::PluginModule）
		\li Visindigo::General::PackageManager （等同于Visindigo::General::PluginManager）
		\endlist

		这样，您可称呼它们为“应用程序包”、“应用程序包模块”和“应用程序包管理器”，而不是“插件”、“插件模块”和“插件管理器”，
		以避免混淆。

		稍后，您可以调用VIApplication::setMainPlugin()函数将您的插件类设置为应用程序的主插件（主应用程序包）。具体
		调用顺序请参考VIApplication类的文档说明。

		\section1 作为程序插件

		要开发一个Visindigo插件，您首先需要创建一个继承自Visindigo::General::Plugin类的派生类，并实现名为VisindigoPluginMain的函数。这个
		函数只做一件事情：新建您的插件类，并且返回它，例如：
		\code
			extern "C" YSSCore::Editor::Plugin* VisindigoPluginMain() {
				return new YourPluginClass();
			}
		\endcode
		\warning 请注意，VisindigoPluginMain函数必须是extern "C"的，否则Visindigo将无法找到它。

		Visindigo会通过此函数取得您的插件实例，并且在适当的时候调用它的各个函数。
	
		但是，若要使Yayin Story Studio能够找到您的插件，您还需要至少三步操作：

		1. 在VIApplication的envConfig中指定的插件存放目录下创建存放插件的文件夹，命名可以任选，最好为您的插件ID。
		\note 默认值是"./user_data/plugins"，您可以通过调用VIApplication::setEnvConfig()函数更改它。

		2. 确保您编译得到的动态链接库文件的后缀名为.vpl（这是全平台统一的），并且放在您的插件文件夹中。

		3. 在您的插件文件夹中新建一个与您插件文件同名的json文件，例如“yourplugin.vpl.json"，并做如下编辑：
		\badcode
			{
				"ID": "YourPluginID",
				"Depend": [
					“PluginID1",
					“PluginID2"
				]
			}
		\endcode

		Visindigo在加载插件时会首先搜索所有vpl文件，并且依赖这个与其同名的JSON文件分析插件的依赖信息并决定加载顺序。
		json文件中的ID必须和代码中实际设置的ID一致，否则Visindigo将无法找到您的插件。Depend字段是一个字符串数组，
		表示您的插件依赖的其他插件的ID。Visindigo会据此调整加载顺序，即在加载插件时，先加载这些依赖的插件，然后再加载您的插件。

		请注意，Visindigo无法处理循环依赖。当两个插件相互依赖时，将具有相同的优先等级，Visindigo无法确定其加载顺序。
		从源码实现的角度来看，应该会先加载首字母较小的那个插件，但这属于未定义行为，因此请避免出现循环依赖。

		\note 如果您的插件不依赖其他插件，可以将Depend字段省略。

		考虑到C++的内存使用极其自由，用户编写代码也极其自由，因此Visindigo不保证任何插件
		逻辑的异常安全。任何执行插件函数过程中遭遇的异常都会导致Visindigo直接崩溃。
	*/

	/*! 
		\typedef Visindigo::General::Package
		\relates Visindigo::General::Plugin
		\since Visindigo 0.13.0

		作为Visindigo::General::Plugin类的别名，方便用户将其称为“应用程序包”。
	*/

	/*! 
		\typedef Visindigo::General::PackageModule
		\relates Visindigo::General::PluginModule
		\since Visindigo 0.13.0

		作为Visindigo::General::PluginModule类的别名，方便用户将其称为“应用程序包模块”。
	*/

	/*!
		\typedef Visindigo::General::PackageManager
		\relates Visindigo::General::PluginManager
		\since Visindigo 0.13.0

		作为Visindigo::General::PluginManager类的别名，方便用户将其称为“应用程序包管理器”。
	*/

	/*!
		\a abiVersion 插件的ABI版本
		\a extID 插件的扩展ID
		\a parent 插件的父对象
		\since Visindigo 0.13.0

		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时Visindigo对应的ABI版本。

		插件的扩展ID，是提供给插件扩展开发者的一个标识符，用于确定该插件到底属于哪一类插件。

		例如，如果您继承了Plugin并实现了一个Plugin的扩展（姑且称之为PluginV2），然后将PluginV2提供给他人继续开发，
		则您的PluginV2的默认构造函数中，就可以将extID设置为"Visindigo_PluginV2"，以便后续开发者能够识别这是一个PluginV2类型的插件。

		这已经有所实践。在Yayin Story Studio中，插件的extID即为"YayinStoryStudio"，标识这是一个继承了YSSCore::Editor::EditorPlugin
		所实现的插件。

		\warning 请尤其注意，abiVersion并不是该插件的版本号，也不是Visindigo的API版本。它是Visindigo的ABI版本，它已经
		被默认填充了。
	*/
	Plugin::Plugin(Visindigo::General::Version abiVersion, QString extID, QObject* parent) : QObject(parent) {
		d = new Visindigo::__Private__::PluginPrivate();
		d->ABIVersion = abiVersion;
		d->PluginExtensionID = extID;
		d->Logger = new Logger("UnnamedPlugin");
		d->q = this;
	}
	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	Plugin::~Plugin() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置插件为测试模式。只有在测试模式下，onTest函数才会被调用。
	*/
	void Plugin::setTestEnable() {
		d->TestEnable = true;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件是否为测试模式
	*/
	bool Plugin::isTestEnable() const {
		return d->TestEnable;
	}

	/*!
		\fn void Visindigo::General::Plugin::onPluginEnable()
		\since Visindigo 0.13.0
		启用插件时调用此函数。大部分涉及到Visindigo内部资源调用的初始化都应该在此函数中进行。

		如果在构造函数中初始化，则有可能因为Visindigo尚未完全初始化而导致崩溃。此外，此函数
		还会被Visindigo捕获异常并输出到日志中。
	*/

	/*!
		\fn void Visindigo::General::Plugin::onPluginDisbale()
		\since Visindigo 0.13.0
		禁用插件时调用此函数。大部分涉及到Visindigo内部资源调用的清理都应该在此函数中进行。
		如果在析构函数中清理，则有可能因为Visindigo已经开始析构而导致崩溃。此外，此函数
		还会被Visindigo捕获异常并输出到日志中。
	*/

	/*!
		\fn void Visindigo::General::Plugin::onApplicationInit()
		\since Visindigo 0.13.0
		当整个应用程序所有的插件都被启用后调用此函数。此时，您可以安全地调用其他插件的功能。

		如果您的插件需要与其他插件交互，则应在此函数中进行相关操作，而不是在onPluginEnable()中进行。
	*/

	/*!
		\fn void Visindigo::General::Plugin::onTest()
		\since Visindigo 0.13.0
		如果插件被设置为测试模式（通过调用setTestEnable()函数），则在所有插件的onApplicationInit()函数调用完毕后调用此函数。

		您可以在此函数中执行一些测试代码，例如输出日志信息、弹出对话框等。
	*/

	/*!
		\since Visindigo 0.13.0
		return 插件的ID
	*/
	QString Plugin::getPluginID() const {
		return d->PluginID;
	}
	/*!
		\since Visindigo 0.13.0
		return 插件的名称
	*/
	QString Plugin::getPluginName() const {
		return d->PluginName;
	}
	/*!
		\since Visindigo 0.13.0
		return 插件的作者，这是一个列表。
	*/
	QStringList Plugin::getPluginAuthor() const {
		return d->PluginAuthor;
	}
	/*!
		\since Visindigo 0.13.0
		return 插件的根目录
	*/
	QDir Plugin::getPluginFolder() const {
		return d->PluginFolder;
	}
	/*!
		\since Visindigo 0.13.0
		return 插件的设置
	*/
	Visindigo::Utility::JsonConfig* Plugin::getPluginConfig() {
		return &(d->Config);
	}

	/*!
		\since Visindigo 0.13.0
		return 插件包含的模块列表
	*/
	QList<PluginModule*> Plugin::getModules() const {
		return d->Modules;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的版本号
	*/
	Visindigo::General::Version Plugin::getPluginVersion() const {
		return d->PluginVersion;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的ABI版本号
	*/
	Visindigo::General::Version Plugin::getPluginABIVersion() const {
		return d->ABIVersion;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的扩展ID
	*/
	QString Plugin::getPluginExtensionID() const {
		return d->PluginExtensionID;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的日志记录器
	*/
	Visindigo::General::Logger* Plugin::getLogger() const {
		return d->Logger;
	}

	/*!
		\since Visindigo 0.13.0
		\a id 插件的ID
		设置插件的ID
	*/
	void Plugin::setPluginID(const QString& id) {
		d->PluginID = id;
	}
	/*!
		\since Visindigo 0.13.0
		\a name 插件的名称
		设置插件的名称
	*/
	void Plugin::setPluginName(const QString& name) {
		d->PluginName = name;
		d->Logger->setNamespace(name);
	}
	/*!
		\since Visindigo 0.13.0
		\a author 插件的作者
		设置插件的作者
	*/
	void Plugin::setPluginAuthor(const QStringList& author) {
		d->PluginAuthor = author;
	}
	/*!
		\since Visindigo 0.13.0
		\a version 插件的版本号
		设置插件的版本号
	*/
	void Plugin::setPluginVersion(const Visindigo::General::Version& version) {
		d->PluginVersion = version;
	}
	
	/*!
		\since Visindigo 0.13.0
		\a module 要注册的模块
		注册一个编辑器模块
	*/
	void Plugin::registerPluginModule(PluginModule* module) {
		d->Modules.append(module);
	}

	/*!
		\since Visindigo 0.13.0
		\a translator 要注册的翻译器
		注册一个翻译器
	*/
	void Plugin::registerTranslator(Visindigo::General::Translator* translator) {
		// NOTICE: Translator has not extend PluginModule yet, need to be changed later
		Visindigo::General::TranslationHost::getInstance()->active(translator);
	}

	/*!
		\since Visindigo 0.13.0
		\a schemeFilePath 颜色方案文件路径
		注册一个颜色方案
	*/
	void Plugin::registerColorScheme(const QString& schemeFilePath) {
		VISTM->pluginRegisterColorScheme(this, Visindigo::Utility::FileUtility::readAll(schemeFilePath));
	}

	/*!
		\since Visindigo 0.13.0
		\a templateFilePath 样式模板文件路径
		注册一个样式模板
	*/
	void Plugin::registerStyleTemplate(const QString& templateFilePath) {
		VISTM->pluginRegisterStyleTemplate(this, Visindigo::Utility::FileUtility::readAll(templateFilePath));
	}
}