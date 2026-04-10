#include <QtCore/qstring.h>
#include "General/TranslationHost.h"
#include "General/private/Plugin_p.h"
#include "General/Plugin.h"
#include "General/PluginModule.h"
#include "General/Version.h"
#include "General/Logger.h"
#include "Widgets/ThemeManager.h"
#include "Utility/FileUtility.h"
#include "General/Placeholder.h"
#include "General/CommandHost.h"
#include "General/CommandHandler.h"
#include "General/Log.h"

namespace Visindigo::__Private__ {
	void PluginPrivate::initializePluginFolder(const QDir& baseDir) {
		PluginFolder = QDir(baseDir.filePath(PluginID));
		if (!PluginFolder.exists()) {
			PluginFolder.mkpath(".");
		}
		loadConfig();
	}

	void PluginPrivate::setPluginLoadType(Visindigo::General::Plugin::LoadType loadType) {
		LoadType = loadType;
	}

	void PluginPrivate::loadConfig() {
		if (not Visindigo::Utility::FileUtility::isFileExist(PluginFolder.filePath("config.json"))) {
			if (Visindigo::Utility::FileUtility::isFileExist(":/resource/" % PluginID % "/config.json")) {
				Visindigo::Utility::FileUtility::copyFile(":/resource/" % PluginID % "/config.json", PluginFolder.filePath("config.json"));
			}
			else {
				vgWarning << "No default config found for plugin " << PluginID << ", creating an empty config.";
				Visindigo::Utility::FileUtility::saveAll(PluginFolder.filePath("config.json"), "{}");
			}
		}
		Config.parse(Visindigo::Utility::FileUtility::readAll(PluginFolder.filePath("config.json")));
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
			q->onPluginDisable();
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
		\ingroup VIPlugin
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
		\li onPluginDisable()：当插件或应用程序被禁用时调用。在此函数中，您应进行插件或应用程序的清理工作，大部分析构造操作应该在此进行。
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
		\enum Visindigo::General::Plugin::LoadType
		\since Visindigo 0.13.0
		\value Unknown 插件的加载类型未知
		\value FromDisk 插件是从磁盘加载的，这些插件在PluginManager::getLoadedPlugins中可用
		\value FromMemory 插件是从内存加载的，即应用程序在编译时直接链接的插件，这些插件在
			VIApplication::getPackages()中可用。这个函数不会返回主插件。
		\value MainPlugin 插件是作为应用程序的主插件加载的，这插件在VIApplication::getMainPlugin()中可用
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
		\a apiVersion 插件的API版本
		\a abiVersion 插件的ABI版本
		\a extID 插件的扩展ID
		\a parent 插件的父对象
		\since Visindigo 0.13.0


		对于abiVersion，其已经在头文件中被默认填充为Compiled_YSSABI_Version，代表您的插件编译时Visindigo对应的ABI版本。

		apiVersion和abiVersion拥有相同行为。它们都是用来确保插件与Visindigo之间的二进制接口兼容性的。Visindigo在加载插件时会检查
		API和ABI版本，以确保插件能够正确地与当前运行的Visindigo版本进行交互。

		插件的扩展ID，是提供给插件扩展开发者的一个标识符，用于确定该插件到底属于哪一类插件。

		例如，如果您继承了Plugin并实现了一个Plugin的扩展（姑且称之为PluginV2），然后将PluginV2提供给他人继续开发，
		则您的PluginV2的默认构造函数中，就可以将extID设置为"Visindigo_PluginV2"，以便后续开发者能够识别这是一个PluginV2类型的插件。

		这已经有所实践。在Yayin Story Studio中，插件的extID即为"YayinStoryStudio"，标识这是一个继承了YSSCore::Editor::EditorPlugin
		所实现的插件。

		\warning 请尤其注意，abiVersion并不是该插件的版本号，也不是Visindigo的API版本。它是Visindigo的ABI版本，它已经
		被默认填充了。
	*/
	Plugin::Plugin(Visindigo::General::Version apiVersion, Visindigo::General::Version abiVersion, QString extID, QObject* parent) : QObject(parent) {
		d = new Visindigo::__Private__::PluginPrivate();
		d->APIVersion = apiVersion;
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
		\fn void Visindigo::General::Plugin::onPluginDisable()
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
		return 插件的描述信息
	*/
	QString Plugin::getPluginDescription() const {
		return d->PluginDescription;
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
		保存插件的设置到磁盘
	*/
	void Plugin::savePluginConfig() {
		Visindigo::Utility::FileUtility::saveAll(d->PluginFolder.filePath("config.json"), d->Config.toString());
	}

	/*!
		\since Visindigo 0.13.0
		从磁盘重新加载插件的设置。

		如果你的插件实现了getConfigWidget，允许用UI配置可视化文件，那么就强烈建议你的
		配置页面在保存之后调用此函数以确保加载到内存中的设置与磁盘上的一致，
		避免出现配置页面显示的设置与实际设置不一致的情况。
	*/
	void Plugin::reloadPluginConfig() {
		d->loadConfig();
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的加载方式
	*/
	Plugin::LoadType Plugin::getPluginLoadType() const {
		return d->LoadType;
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
		\a id 模块的ID
		return 插件中ID为\a id的模块，如果没有找到，则返回nullptr

		请注意，出于从ID搜索模块可能并不常用考虑，在注册模块到插件时，
		并不会立即将他们存入Map中以节省空间和提高性能，因此使用此
		函数第一次搜索某个ID时会很慢，因为插件需要现场遍历一次模块列表。

		但搜索到后会将结果存入Map中，因此第二次搜索同一个ID时性能会相对较好。
	*/
	PluginModule* Plugin::getModuleByID(const QString& id) const {
		if (d->ModuleIDMap.contains(id)) {
			return d->ModuleIDMap.value(id);
		}
		for (auto module : d->Modules) {
			if (module->getModuleID() == id) {
				d->ModuleIDMap.insert(id, module);
				return module;
			}
		}
		return nullptr;
	}

	/*!
		\since Visindigo 0.13.0
		\a typeID 特定类型的ID
		return 插件中全部类型ID为\a typeID的模块列表，如果没有找到，则返回一个空列表

		请注意，出于从类型ID搜索模块可能并不常用考虑，在注册模块到插件时，
		并不会立即将他们存入Map中以节省空间和提高性能，因此使用此
		函数第一次搜索某个类型ID时会很慢，因为插件需要现场遍历一次模块列表。

		但搜索到后会将结果存入Map中，因此第二次搜索同一个类型ID时性能会相对较好。
	*/
	QList<PluginModule*> Plugin::getModuleByTypeID(const QString& typeID) const {
		if (d->ModuleTypeIDMap.contains(typeID)) {
			return d->ModuleTypeIDMap.value(typeID);
		}
		QList<PluginModule*> result;
		for (auto module : d->Modules) {
			if (module->getModuleTypeID() == typeID) {
				result.append(module);
			}
		}
		d->ModuleTypeIDMap.insert(typeID, result);
		return result;
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
		return 插件的API版本号
	*/
	Visindigo::General::Version Plugin::getPluginAPIVersion() const {
		return d->APIVersion;
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
		return 插件的配置界面，如果插件没有提供配置界面，则默认返回nullptr

		关于此虚函数建议如下原则实现：
		
		懒初始化您的插件配置页面，即在第一次有人调用此函数前，
		不要创建您的插件配置页面，这有助于节省内存和提高性能。
		与此同时，建议持久化存储该指针并在disable时再销毁它。
		调用此函数以获取指针的调用方不会接管该指针的生命周期。

		与此同时，不要对该函数调用时机和目的做任何特殊假定
		（例如认为它只会在即将需要显示该页面前调用从而做一些
		显示前的准备操作），这个函数很有可能在Visindigo检查
		插件配置页面是否可用时或其他情况下被调用。如果您需要一些
		展示前工作，请考虑重写或拦截该页面的showEvent。
	*/
	QWidget* Plugin::getConfigWidget() {
		return nullptr;
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
		\a description 插件的描述信息
		设置插件的描述信息
	*/
	void Plugin::setPluginDescription(const QString& description) {
		d->PluginDescription = description;
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
		注册一个插件模块
	*/
	void Plugin::registerPluginModule(PluginModule* module) {
		d->Modules.append(module);
		if (d->ModuleTypeIDMap.contains(module->getModuleTypeID())) {
			d->ModuleTypeIDMap[module->getModuleTypeID()].append(module);
		}
		if (module->getModuleTypeID() == QString(VIModuleType_Translator)) {
			TranslationHost::getInstance()->registerTranslator(dynamic_cast<Translator*>(module));
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_PlaceholderProvider)) {
			PlaceholderManager::getInstance()->registerProvider(dynamic_cast<PlaceholderProvider*>(module));
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_CommandHandler)) {
			CommandHost::getInstance()->registerCommand(dynamic_cast<CommandHandler*>(module));
			dynamic_cast<CommandHandler*>(module)->enable();
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a module 要注销的模块
		注销一个插件模块
	*/
	void Plugin::unregisterPluginModule(PluginModule* module) {
		d->Modules.removeAll(module);
		d->ModuleIDMap.remove(module->getModuleID());
		if (d->ModuleTypeIDMap.contains(module->getModuleTypeID())) {
			d->ModuleTypeIDMap[module->getModuleTypeID()].removeAll(module);
		}
		if (module->getModuleTypeID() == QString(VIModuleType_Translator)) {
			TranslationHost::getInstance()->unregisterTranslator(dynamic_cast<Translator*>(module));
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_PlaceholderProvider)) {
			
		}
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