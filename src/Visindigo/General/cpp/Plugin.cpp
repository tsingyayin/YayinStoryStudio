#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include "General/CommandHandler.h"
#include "General/CommandHost.h"
#include "General/Log.h"
#include "General/Logger.h"
#include "General/Placeholder.h"
#include "General/Plugin.h"
#include "General/PluginManager.h"
#include "General/PluginModule.h"
#include "General/PluginPermission.h"
#include "General/private/Plugin_p.h"
#include "General/TranslationHost.h"
#include "General/Version.h"
#include "General/Exception.h"
#include "Utility/FileOperation.h"
#include "Utility/FileUtility.h"
#include "Widgets/ThemeManager.h"

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
		const QString configPath = PluginFolder.filePath("config.json");
		if (not Visindigo::Utility::FileUtility::isFileExist(configPath)) {
			const QString defaultConfigPath = ":/resource/" % PluginID % "/config.json";
			if (Visindigo::Utility::FileUtility::isFileExist(defaultConfigPath)) {
				Visindigo::Utility::FileOperation::ErrorCode copyResult = Visindigo::Utility::FileOperation::copyFile(defaultConfigPath, configPath);
				if (copyResult != Visindigo::Utility::FileOperation::Success) {
					vgErrorF << "Failed to copy default config for plugin " << PluginID
						<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(copyResult);
				}
			}
			else {
				vgWarning << "No default config found for plugin " << PluginID << ", creating an empty config.";
				Visindigo::Utility::FileOperation::ErrorCode saveResult = Visindigo::Utility::FileOperation::saveAll(configPath, "{}");
				if (saveResult != Visindigo::Utility::FileOperation::Success) {
					vgErrorF << "Failed to create empty config for plugin " << PluginID
						<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(saveResult);
				}
			}
		}
		Visindigo::Utility::FileOperation::Errorable<QString> configResult = Visindigo::Utility::FileOperation::readAll(configPath);
		if (not configResult) {
			vgErrorF << "Failed to read config for plugin " << PluginID << ": " << configPath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(configResult.error());
			return;
		}
		Config.parse(configResult.value());
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
		\brief 插件的基类，同时也是应用程序的基类.
		\since Visindigo 0.13.0

		Visindigo::General::Plugin 是Visindigo插件系统中最核心的类型。无论您要编写一个可以被他人加载的插件，
		还是要编写一个完整的应用程序，都必须实现此类的一个派生类。Visindigo将插件与应用程序按等同逻辑处理，
		唯一区别在于它们的加载与卸载时机和先后顺序，这在Visindigo::General::VIApplication的文档中有详细说明。

		\section1 概念与定位

		\section2 什么是插件
		插件是Visindigo运行时装载的功能单元。每一个插件都是一个Plugin的派生类实例，插件管理器会在恰当时机
		依次调用它的各个生命周期函数，从而把插件的初始化、测试和清理纳入整个应用程序的启动和退出流程。

		无论是程序内建的插件还是从磁盘加载的插件，都遵循同一套生命周期与元数据规则，
		它们在运行时的差异只体现在getPluginLoadType()函数的返回值上。

		\section2 插件与插件模块
		一个插件可以包含零个或多个插件模块（Visindigo::General::PluginModule）。模块是插件内部的功能单元，
		例如翻译器、占位符提供者、命令处理程序，以及在编辑器层中扩展出来的文件服务器、语言服务器等。

		模块在构造时会以所属插件作为其父对象，因此模块与插件的生命周期是绑定的：插件被销毁时，其模块会
		被Qt的对象树自动销毁。插件可以通过registerPluginModule()注册模块，注册后部分类型的模块会被
		自动登记到Visindigo对应的管理器中，详见"插件模块"一节。

		值得指出的是，插件模块只是Visindigo推荐的插件内部功能划分与生命周期协调方式，不是必须选项。

		\section2 插件与应用程序
		应用程序本身就是一个插件：它同样需要从Plugin派生，并同样具有生命周期函数。按加载来源，
		一次运行中的插件可以分为三类：

		\list
		\li 从磁盘加载的插件，由PluginManager扫描VIApplication::EnvKey::PluginFolderPath指定的目录得到；
		\li 应用程序的主插件，由VIApplication::setMainPlugin()指定；
		\li 与应用程序一同编译的插件，由VIApplication::addDependencyPlugin()登记。
		\endlist

		这三类插件的启用顺序是：依赖插件、主插件、磁盘插件；禁用顺序则与启用顺序基本相反，
		即先禁用磁盘插件，再禁用主插件，最后禁用依赖插件。此外，Visindigo自身的内核插件与部件插件
		会先于上述所有插件启用。完整的顺序与时机说明请参考Visindigo::General::VIApplication的文档。

		因此，编写插件与编写应用程序所需遵循的原则完全一致，您在阅读本文档时把"插件"替换为"应用程序"
		不会产生任何歧义。

		\section2 别名：使用"应用程序包"一词
		如果您不习惯用"插件"称呼您的应用程序，可以使用Package.h中提供的别名：
		\list
		\li Visindigo::General::Package，等同于Visindigo::General::Plugin
		\li Visindigo::General::PackageModule，等同于Visindigo::General::PluginModule
		\li Visindigo::General::PackageManager，等同于Visindigo::General::PluginManager
		\endlist

		这样，您就可以称它们为"应用程序包"、"应用程序包模块"和"应用程序包管理器"，而不是"插件"、
		"插件模块"和"插件管理器"，以避免混淆。这些别名只是using声明，使用它们不会产生任何额外的开销。

		Package系列叫法基本上是前Visindigo 0.10时代留下来的习惯称呼。

		稍后，您可以调用VIApplication::setMainPlugin()函数将您的插件类设置为应用程序的主插件（主应用程序包）。
		具体调用顺序请参考VIApplication类的文档说明。

		\section1 生命周期

		\section2 生命周期阶段一览
		Plugin类最重要的函数有五个，是用户必须实现的函数，即构造函数、onPluginEnable()、onPluginDisable()、
		onApplicationInit()和onTest()。无论是开发应用程序还是插件，这五个函数都遵循相同的逻辑与原则：

		\list
		\li 构造函数：只用于设置本插件或本应用程序的基础信息，如setPluginName()、setPluginAuthor()、
		setPluginVersion()等。除此之外不应执行任何其他操作，尤其是与Visindigo或其他插件交互的操作，
		因为在构造函数执行时，Visindigo尚未准备好全部功能，您极有可能遭遇nullptr。
		\li onPluginEnable()：当插件或应用程序被启用时调用。在此函数中，您应进行插件或应用程序的初始化工作，
		大部分构造操作应该在此进行。
		\li onApplicationInit()：当整个应用程序所有的插件都被启用后，它会被按一定顺序调用。您可以在此函数中
		执行一些需要所有插件都已启用后才能进行的操作。
		\li onTest()：如果您的插件或应用程序被设置为启用测试功能（通过调用setTestEnable()函数），
		则在所有插件的onApplicationInit()函数调用完毕后，它会被按一定顺序调用。您可以在此函数中执行一些测试代码。
		\li onPluginDisable()：当插件或应用程序被禁用时调用。在此函数中，您应进行插件或应用程序的清理工作，
		大部分析构操作应该在此进行。
		\li 析构函数：不推荐实现这个函数，保留为编译器默认值即可。如果您确实需要实现它，请确保在析构函数中
		不与Visindigo或其他插件交互。
		\endlist

		\section2 构造函数
		构造函数是唯一一个在插件实例被创建时立即执行的函数，它的职责仅限于设置插件的基础信息：
		插件ID、版本号、名称、作者、描述和所需的权限。此时Visindigo的其余部分可能尚未初始化完毕，
		因此绝对不要在这里访问其他插件、管理器或UI资源。

		构造函数会校验插件ID是否合法，并在ID不合法时抛出异常（见"插件ID"一节）。

		\section2 onPluginEnable()
		启用阶段调用的函数，是插件进行初始化的主要场所：创建模块、注册主题资源、读取配置、
		建立与其他插件无关的资源等。只要操作不需要依赖其他插件，就应该放在这里而不是onApplicationInit()中。

		Visindigo会捕获这个函数抛出的异常并记录到日志中，此时插件会被立即调用onPluginDisable，
		但插件对象本身仍然保留在内存中。

		\section2 onApplicationInit()
		所有插件都被启用之后调用的函数。此时您可以安全地调用其他插件的功能，因此需要跨插件协作的操作
		应该放在这里，而不是onPluginEnable()中。

		\section2 onTest()
		测试阶段的函数，只有在插件通过setTestEnable()声明了自己需要被测试时才会被调用。它通常用于
		输出诊断信息、弹出自检对话框或运行临时性的测试代码。

		\section2 onPluginDisable()
		禁用阶段调用的函数，是插件进行清理的主要场所：销毁模块、断开信号连接、释放资源等。
		与构造函数对应的理由相同，不要试图在析构函数中完成这些工作，因为此时Visindigo可能已经开始析构，
		您极有可能遭遇崩溃。

		Visindigo会捕获这个函数抛出的异常并记录到日志中。

		\section2 析构函数
		不推荐实现析构函数，保留编译器默认实现即可。如果确实需要实现，请确保它不访问其他插件、
		不访问Visindigo的全局单例，也不执行任何UI操作；同时应当保持它是virtual的，因为插件对象总是通过
		基类指针被销毁。

		\section2 异常处理
		考虑到C++的内存使用极其自由，用户编写代码也极其自由，因此Visindigo不尝试实现任何隔离机制，
		也不保证任何插件逻辑的异常安全。

		框架只在插件启用和禁用这两个阶段捕获异常（即onPluginEnable()与onPluginDisable()），
		除此之外，任何执行插件函数过程中遭遇的异常都会导致Visindigo直接崩溃。

		\section1 标识与元数据

		\section2 插件ID
		插件ID是插件在整个应用程序中的唯一标识，它在构造函数的第一个参数中给出，并且在插件的整个生命周期内不可更改。

		自0.17.0起，Visindigo要求插件ID必须严格符合逆序域名规则，且除了点之外，只允许使用大小写字母、数字
		（数字不能在每段的开头）和下划线。即插件ID必须符合正则表达式：
		\code
			^[a-zA-Z][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9_]*)*$
		\endcode

		如果不符合此规则，插件会在构造时抛出Visindigo::General::Exception异常，导致插件无法被加载。

		插件ID还决定了以下内容，因此请谨慎选择：
		\list
		\li 插件配置文件夹的名称（见"插件文件夹"一节）；
		\li 插件在元数据json中必须声明的ID字段；
		\li 插件专用Logger的命名空间。
		\endlist

		\section2 版本：API版本、ABI版本与插件版本
		Plugin类同时涉及三个互不相同的版本号，请不要混淆：

		\list
		\li API版本：标识Visindigo的公开接口版本，传入构造函数的apiVersion参数，默认值为Compiled_VIAPI_Version。
		\li ABI版本：标识Visindigo的二进制接口版本，传入构造函数的abiVersion参数，默认值为Compiled_VIABI_Version。
		它已经在头文件中被默认填充，请不要手动修改。请尤其注意，ABI版本既不是该插件的版本号，也不是Visindigo的API版本。
		\li 插件版本：插件自身的版本号，通过setPluginVersion()设置、通过getPluginVersion()获取。
		它不参与兼容性检查，仅供显示和插件自身逻辑使用。
		\endlist

		API版本与ABI版本的行为一致，二者都是用来保证插件与Visindigo之间的二进制接口兼容性的。
		Visindigo在加载插件时会检查这两个版本，若不兼容则拒绝加载该插件并记录错误。

		\section2 名称、作者与描述
		插件的名称、作者和描述通过setPluginName()、setPluginAuthor()和setPluginDescription()设置，
		它们只影响显示，其中名称和描述还可以按VI18N的格式给出翻译引用名称，
		从而通过getPluginNameI18N()和getPluginDescriptionI18N()取得国际化后的内容。

		请注意，setPluginName()同时也会把插件专用Logger的命名空间设置为国际化后的名称，
		以便日志输出中能直接看到可读的插件名。

		\section2 扩展ID（extID）
		扩展ID是提供给插件扩展开发者的一个标识符，用于确定该插件到底属于哪一类插件。

		例如，如果您继承了Plugin并实现了一个Plugin的扩展（姑且称之为PluginV2），然后将PluginV2提供给
		他人继续开发，则您的PluginV2的默认构造函数中，就可以将extID设置为"Visindigo_PluginV2"，
		以便后续开发者能够识别这是一个PluginV2类型的插件。

		这已经有所实践。在Yayin Story Studio中，插件的extID即为"YayinStoryStudio"，
		标识这是一个继承了YSSCore::Editor::EditorPlugin所实现的插件。未显式指定时，
		扩展ID的默认值为"Visindigo_Base"。

		\section2 加载类型
		getPluginLoadType()返回插件是如何被加载的，它决定了该插件在哪个集合中可被检索到：
		\list
		\li FromDisk：从磁盘加载的插件，可在PluginManager::getLoadedPlugins()中取得；
		\li FromMemory：随应用程序一同编译的插件，可在VIApplication::getDependencyPlugins()中取得。
		该函数不会返回主插件；
		\li MainPlugin：作为应用程序主插件加载的插件，可在VIApplication::getMainPlugin()中取得。
		\endlist

		\section1 磁盘布局与加载

		\section2 插件文件夹
		插件文件夹是指插件用于存放用户数据的文件夹，是VIApplication::EnvKey::ConfigPath中指定的配置存放目录下，
		以插件ID命名的文件夹。按插件的加载类型不同，它的具体位置也不同：
		\list
		\li 从磁盘加载的插件：配置目录下的plugins文件夹；
		\li 应用程序的主插件：配置目录下的program文件夹；
		\li 随应用程序编译的依赖插件：配置目录下的depends文件夹。
		\endlist

		插件文件夹由Visindigo在插件被启用前创建，插件的配置文件config.json也位于其中，
		因此请通过getPluginFolder()获取该路径，不要自行拼接。

		\section2 插件二进制文件夹
		插件二进制文件夹是扫描到插件时，其二进制文件所在的文件夹，可通过getPluginBinaryFolder()获取，
		它实际转发自PluginManager::getPluginBinaryFolder()。

		注意与插件文件夹的区别：插件文件夹用于存放用户数据，插件二进制文件夹用于存放插件对应在
		磁盘中的二进制实体。

		对于从磁盘扫描并自动加载的插件，二进制文件夹就是扫描到的插件二进制文件所在位置。对于主插件、依赖插件，
		则为程序所在文件夹。不过由于PluginManager还支持显式指定其他手动加载位置，因此部分加载情况下的二进制文件夹较为特殊，
		详见Visindigo::General::PluginManager类中相关函数的说明，下文也对此有所解释。

		\section2 插件文件与元数据json
		要开发一个从磁盘加载的Visindigo插件，您首先需要创建一个继承自Visindigo::General::Plugin类的派生类，
		并实现名为VisindigoPluginMain的函数。这个函数只做一件事情：新建您的插件类，并且返回它，例如：
		\code
			extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain() {
				return new YourPluginClass();
			}
		\endcode

		\warning 请注意，VisindigoPluginMain函数必须是extern "C"的，否则Visindigo将无法找到它。

		Visindigo会通过此函数取得您的插件实例，并且在适当的时候调用它的各个函数。但是，
		若要使Visindigo能够找到您的插件，您还需要至少三步操作：

		\list
		\li 1. 在VIApplication的envConfig中指定的插件存放目录下创建存放插件的文件夹，命名可以任选，最好为您的插件ID。
		\note 加载路径的默认值是"./user_data/plugins"，您可以通过调用VIApplication::setEnvConfig()函数更改它。
		\li 2. 确保您编译得到的动态链接库文件的后缀名为.vpl（这是全平台统一的），并且放在您的插件文件夹中。
		\li 3. 在您的插件文件夹中新建一个与您插件文件同名的json文件，例如"yourplugin.vpl.json"，并做如下编辑：
		\badcode
			{
				"ID": "YourPluginID",
				"Dependencies": [
					"PluginID1",
					"PluginID2"
				]
			}
		\endcode
		\endlist

		Visindigo在加载插件时会首先搜索所有vpl文件，并且依赖这个与其同名的JSON文件分析插件的依赖信息并决定加载顺序。
		json文件中的ID必须和代码中实际设置的ID一致，否则Visindigo将无法找到您的插件。Dependencies字段是一个
		字符串数组，表示您的插件依赖的其他插件的ID。Visindigo会据此调整加载顺序，即在加载插件时，
		先加载这些依赖的插件，然后再加载您的插件。

		请注意，Visindigo无法处理循环依赖。当两个插件相互依赖时，将具有相同的优先等级，Visindigo无法确定其加载顺序。
		从源码实现的角度来看，应该会先加载首字母较小的那个插件，但这属于未定义行为，因此请避免出现循环依赖。

		\note 如果您的插件不依赖其他插件，可以将Dependencies字段省略。

		\section2 独立外部依赖（DependLibs）
		如果您的插件依赖于其他非Visindigo插件的外部库，您需要确保这些库在Visindigo运行时能够被找到。在0.17.0以前，
		只能将这些库直接塞进Visindigo应用程序的运行目录下，或将它们放在系统的PATH环境变量中。
		自0.17.0起，作为外部依赖的库可以直接放在插件的文件夹中，并在插件的json文件中指定它们的路径，例如：
		\badcode
			{
				"ID": "YourPluginID",
				"Dependencies": [
					"PluginID1",
					"PluginID2"
				],
				"DependLibs": [
					"libYourExternalDependency"
				]
			}
		\endcode

		这里的路径是插件所在文件夹开始的相对路径，您可以指定子文件夹。此外，不必指定后缀名，
		Visindigo会根据当前平台自动寻找对应的后缀名。因此对于大部分依赖库和插件本体
		一同放置在同一个文件夹里的情况，这里直接写依赖库的主文件名即可。

		此外，由于动态链接库加载强烈依赖操作系统行为，因此可能会造成潜在冲突。例如在Windows上，
		只能依赖dll文件名称区分动态链接库，如果插件A和插件B都依赖同一个完全一样的dll，则不会发生问题，
		但如果两个同名dll版本不一致，或者只是撞名的两个完全不同的dll，则可能会导致Visindigo崩溃。
		虽然在macOS和Linux上允许从不同路径加载同名动态链接库，但这种情况下如果是同名不同版本的动态链接库，
		则也可能因为符号冲突而导致Visindigo崩溃。

		因此，Visindigo采取一个简单粗暴的方案：记录所有已加载外部依赖的名称和依赖库的哈希值，
		如果发现同名的外部依赖具有相同哈希，则会允许加载，但如果发现同名的外部依赖具有不同哈希，
		则会直接终止对第二个尝试加载该外部依赖的插件的加载，并且在日志中记录错误信息。

		请尽量避免这种情况发生——如果您的外部依赖是自己编译的，请尽量使用类似于
		"libYourExternalDependency_v1.0.0.dll"的命名方式，以避免与其他插件的外部依赖发生冲突。

		\section2 特殊的加载路径
		除了由PluginManager自动扫描插件目录之外，还允许通过PluginManager::addPluginLoadPath()和
		PluginManager::addPluginEntryPoint()两个函数手动登记插件，它们用于处理无法被自动扫描覆盖的场景，
		例如插件二进制随应用程序一同打包、由操作系统直接载入内存，但逻辑上确实作为独立插件的情况（Android上即采用这种做法）：
		\code
			extern "C" YSSFileExtAPI Visindigo::General::Plugin* VisindigoPluginMain_YSSFileExt();
			// ...
			pluginManager->addPluginEntryPoint(&VisindigoPluginMain_YSSFileExt,
				QStringLiteral(":/plugins/Plugin_YSSFileExt.vpl.json"));
		\endcode

		这两个函数必须在PluginManager::loadAllPlugin()之前调用，否则会被忽略。此外，这两个函数中插件二进制文件夹的
		行为较为特殊，请参见它们各自的说明。

		\section1 插件配置

		\section2 配置对象
		每个插件都拥有一个独立的配置对象（Visindigo::Utility::JsonConfig），可通过getPluginConfig()取得，
		它会被自动加载和保存到插件文件夹下的config.json中。相关的成员函数有：
		\list
		\li getPluginConfig()：取得配置对象指针，其生命周期由插件管理，调用方不要销毁它；
		\li savePluginConfig()：把内存中的配置写入磁盘；
		\li reloadPluginConfig()：从磁盘重新读取配置，覆盖内存中的内容；
		\li testPluginConfig()：读取一个布尔型配置项的便捷函数。
		\endlist

		\section2 配置界面
		如果插件需要提供配置界面，可以重写getConfigWidget()，Visindigo会在需要时调用它并显示返回的窗口部件。
		实现这个函数时建议遵循以下原则：

		\list
		\li 懒初始化您的插件配置页面，即在第一次有人调用此函数前，不要创建您的插件配置页面，这有助于节省内存和提高性能。
		\li 建议持久化存储该指针，并在禁用时再销毁它。调用此函数以获取指针的调用方不会接管该指针的生命周期。
		\li 不要对该函数调用时机和目的做任何特殊假定（例如认为它只会在即将需要显示该页面前调用，
		从而做一些显示前的准备操作），这个函数很有可能在Visindigo检查插件配置页面是否可用时或其他情况下被调用。
		如果您需要一些展示前工作，请考虑重写或拦截该页面的showEvent。
		\endlist

		由于配置界面可能被用户直接编辑并保存，因此强烈建议您的配置页面在保存之后调用reloadPluginConfig()，
		以确保加载到内存中的设置与磁盘上的一致，避免出现配置页面显示的设置与实际设置不一致的情况。

		\section1 插件模块

		\section2 模块的注册与注销
		插件通过registerPluginModule()注册模块、通过unregisterPluginModule()注销模块，这两个函数都是protected的，
		只能由插件自己在恰当的位置调用。

		模块对象在构造时已经把插件作为自己的父对象，因此模块的销毁由Qt的对象树负责，这两个函数只负责登记关系，
		不会销毁模块对象。注销模块时，模块对象仍然存在，只是不再被插件和管理器所引用。

		\section2 模块的按类型分发
		注册模块时，Visindigo会根据模块的类型ID把它们登记到对应的管理器中：
		\list
		\li VIModuleType_Translator：登记到Visindigo::General::TranslationHost；
		\li VIModuleType_PlaceholderProvider：登记到Visindigo::General::PlaceholderManager；
		\li VIModuleType_CommandHandler：登记到Visindigo::General::CommandHost，并调用其enable()。
		\endlist

		注销模块时，目前只处理上述前两种类型，命令处理程序模块的注销需要插件自行调用其disable()。
		除此之外的类型（例如编辑器层定义的YSSPluginModule_FileServer等）由相应层级自行登记到自己的管理器中，
		Visindigo核心并不感知它们。

		\section2 模块的查询
		getModules()返回全部模块列表。getModuleByID()和getModuleByTypeID()用于按ID或类型ID检索模块。

		请注意，出于从ID搜索模块可能并不常用考虑，在注册模块到插件时，并不会立即将它们存入Map中以节省空间和提高性能，
		因此使用这两个函数第一次搜索某个ID或类型ID时会很慢，因为插件需要现场遍历一次模块列表。
		但搜索到后会将结果存入Map中，因此第二次搜索同一个ID或类型ID时性能会相对较好。

		\section1 主题资源

		\section2 配色方案与样式模板
		插件可以通过registerColorScheme()和registerStyleTemplate()向程序注册配色方案与样式模板，
		这两个函数接受一个文件路径，读取文件内容后交给Visindigo::Widgets::ThemeManager处理。

		由于注册时会把插件对象本身一并交给ThemeManager记录，因此请确保这些资源确实属于本插件；
		程序在需要显示"这个配色方案/样式模板来自哪个插件"时依赖这一记录。

		\section2 注册结果
		注册是可能失败的：文件读取失败、内容不是合法的JSON或VST、缺少SchemeID/Themes/SchemeName等必要字段
		都会导致注册失败。注册失败时Visindigo会在日志中记录原因，但不会抛出异常。

		\section1 权限

		\section2 约定权限系统
		Visindigo的插件系统约定了一个权限系统，以便为部分敏感操作的权限设置统一控制接口。不过我们必须指出，
		由于Visindigo/Qt开发是开放且自由的，因此Visindigo无法保证任何插件都严格遵循其声明的权限来规范自身行为，
		也无法保证任何插件都不会绕过Visindigo的权限系统执行敏感操作——本质上，只要插件愿意，它们可以干任何事情。

		因此，Visindigo的权限系统仅仅是一个约定，旨在为插件开发者提供一个统一的接口来声明和检查权限，
		以给予用户更多的控制权。我们提倡所有插件开发者都遵循这个约定，以便为用户提供一个更安全的插件生态环境。
		如果Visindigo后期开放在线插件市场或其他类似的软件源平台，则不遵守此约定的插件将无法在这些平台上发布。

		值得指出的是，Visindigo插件权限系统对作为主插件或依赖插件的Plugin不进行约束。
		Visindigo::General::PluginPermissionManager::hasPermission()会先向VIApplication核对发起申请的插件是否为
		VIApplication::getMainPlugin()返回的主插件，或包含在VIApplication::getDependencyPlugins()中，若是则直接
		返回true，既不检查其声明过的权限，也不询问用户。它们本身就是应用程序的可信组成部分，其能力边界由应用
		程序自行负责。这一判定依据取自VIApplication的登记结果，而非插件自己声明的getLoadType()。

		\section2 权限列表
		完整的权限列表见Plugin::Permission枚举的说明。其中FileRead和FileWrite是大部分插件唯一需要声明的权限，
		CrossPluginAccess、ProcessAccess、RunAnyCommand等权限则需要格外谨慎地声明。

		\section2 声明权限
		插件应当在构造函数通过setRequiredPermissions()完整声明自己所需的全部权限，
		以便用户在授权时能掌握插件的全部敏感行为。只有在这里声明过的权限才会被PluginPermissionManager检查，
		对于未声明的权限，hasPermission()一律返回false。

		\section2 检查权限
		在访问敏感资源之前，插件应调用hasPermission()检查自己是否拥有对应权限。这个函数会在权限尚未被用户
		决定时向用户发起询问（自动请求权限），因此不要在循环或高频路径中反复调用它，
		而应该在需要使用权限之前检查一次并缓存结果。

		\section1 日志

		每个插件都拥有一个专属的Logger对象（Visindigo::General::Logger），可通过getLogger()取得。
		它的命名空间初始为插件ID，并在setPluginName()被调用时被设置为国际化后的插件名称，
		因此日志输出中能够直接辨认出是哪个插件产生了这条消息。

		插件不需要自行创建其他Logger，也不应当在析构函数中使用Logger。

		\sa Visindigo::General::PluginManager, Visindigo::General::PluginModule, Visindigo::General::VIApplication,
		Visindigo::General::PluginPermissionManager, Visindigo::Widgets::ThemeManager
	*/

	/*!
		\enum Visindigo::General::Plugin::LoadType
		\since Visindigo 0.13.0

		插件的加载类型，描述了插件是被如何加载进应用程序的。

		\value Unknown 插件的加载类型未知
		\value FromDisk 插件是从磁盘加载的，这些插件在PluginManager::getLoadedPlugins中可用
		\value FromMemory 插件是从内存加载的，即应用程序在编译时直接链接的插件，这些插件在
			VIApplication::getDependencyPlugins中可用。这个函数不会返回主插件。
		\value MainPlugin 插件是作为应用程序的主插件加载的，这插件在VIApplication::getMainPlugin中可用
	*/

	/*!
		\enum Visindigo::General::Plugin::Permission
		\since Visindigo 0.17.0

		插件可以声明的权限。这个枚举被用于声明和检查敏感操作的使用权，它只是一个约定，详见Plugin类的文档。

		\value Unknown 未知权限，仅作为零值使用，正常情况下不应出现。
		\value FileRead 读取外部文件。相对的，内部指的是插件配置文件夹、插件二进制文件夹和当前项目目录。
		\value FileWrite 写入外部文件。相对的，内部指的是插件配置文件夹、插件二进制文件夹和当前项目目录。
		\value NetworkLocal 访问本地网络资源，严格指localhost，以及所在局域网的内部资源。
		\value Network 访问网络资源，包括互联网和本地网络资源。
		\value NetworkCellular 允许通过蜂窝网络访问网络资源。
		\value Bluetooth 访问蓝牙资源。
		\value Audio 在设备上播放音频。
		\value Location 访问设备的位置信息。
		\value LaunchExternal 启动任何外部应用，这里主要指启动带有GUI的可执行文件。
		\value ModifySystemSettings 修改系统设置，包括以任何方式改动系统配置文件、注册表、系统服务等。
		\value RunAnyCommand 启动任何外部命令，主要指非GUI的命令行程序或脚本。
		\value ScreenCapture 尝试截取屏幕内容或任何窗口的内容。
		\value BackgroundService 在后台运行与插件本体生命周期不同步的服务或线程，泛指一切后台进程、系统服务或其他可能脱离Visindigo生命周期的程序。
		\value Camera 访问设备的摄像头。
		\value Microphone 访问设备的麦克风。
		\value Clipboard 访问设备的剪贴板。
		\value CrossPluginAccess 访问其他插件的资源或数据。这是FileRead/FileWrite的特例。没有此权限，即使有FileRead/FileWrite权限，也不应访问其他插件的资源或数据。
		\value ProcessAccess 以任何手段访问非应用进程的资源或数据。
	*/

	/*!
		\typedef Visindigo::General::Plugin::Permissions
		\since Visindigo 0.17.0

		Plugin::Permission的位标志组合类型，用于一次性声明或校验多个权限。
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
		\since Visindigo 0.13.0
		\a id 插件的ID
		\a apiVersion 插件的API版本
		\a abiVersion 插件的ABI版本
		\a extID 插件的扩展ID
		\a parent 插件的父对象

		构造一个插件对象。这是派生类构造函数唯一需要调用的构造函数。

		对于abiVersion，其已经在头文件中被默认填充为Compiled_VIABI_Version，代表您的插件编译时Visindigo对应的ABI版本。

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
	Plugin::Plugin(const QString& id, Visindigo::General::Version apiVersion, Visindigo::General::Version abiVersion, QString extID, QObject* parent) : QObject(parent) {
		auto regex = QRegularExpression(R"(^[a-zA-Z][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9_]*)*$)");
		if (not regex.match(id).hasMatch()) {
			VI_Throw(Exception::InvalidArgument, "Plugin ID \"" % id % "\" is invalid. It must follow the reverse domain name convention " 
				"and only contain letters, numbers (not at the beginning of each segment), and underscores.");
		}
		d = new Visindigo::__Private__::PluginPrivate();
		d->PluginID = id;
		d->APIVersion = apiVersion;
		d->ABIVersion = abiVersion;
		d->PluginExtensionID = extID;
		d->Logger = new Logger(d->PluginID);
		d->q = this;
	}
	/*!
		\since Visindigo 0.13.0
		析构插件对象。不推荐实现这个函数，保留编译器默认实现即可；如果确实需要实现，
		请确保它不与Visindigo或其他插件交互。
	*/
	Plugin::~Plugin() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置插件为测试模式。只有在测试模式下，onTest()函数才会被调用。
	*/
	void Plugin::setTestEnable() {
		d->TestEnable = true;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件是否为测试模式。
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
		return 插件的ID。
	*/
	QString Plugin::getPluginID() const {
		return d->PluginID;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的名称。
	*/
	QString Plugin::getPluginName() const {
		return d->PluginName;
	}

	/*!
		\since Visindigo 0.15.0
		return 插件的国际化名称。
	*/
	QString Plugin::getPluginNameI18N() const {
		return VI18N(d->PluginName);
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
		return 插件的描述信息。
	*/
	QString Plugin::getPluginDescription() const {
		return d->PluginDescription;
	}

	/*!
		\since Visindigo 0.15.0
		return 插件的国际化描述信息。
	*/
	QString Plugin::getPluginDescriptionI18N() const {
		return VI18N(d->PluginDescription);
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的配置/数据文件夹，即插件配置文件夹中与插件ID同名的文件夹。
	*/
	QDir Plugin::getPluginFolder() const {
		return d->PluginFolder;
	}

	/*!
		\since Visindigo 0.16.0
		return 插件二进制文件(.vpl)所在的目录。

		注意与getPluginFolder()的区别：getPluginFolder()返回插件的配置/数据目录，
		而getPluginBinaryFolder()返回插件动态库实际存放的目录。

		主插件(MainPlugin)没有对应的二进制文件，此时返回空的QDir。
	*/
	QDir Plugin::getPluginBinaryFolder() const {
		return PluginManager::getInstance()->getPluginBinaryFolder(getPluginID());
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的配置对象指针。其生命周期由插件持有，调用方不要销毁它。
	*/
	Visindigo::Utility::JsonConfig* Plugin::getPluginConfig() {
		return &(d->Config);
	}

	/*!
		\since Visindigo 0.13.0
		把插件的配置保存到磁盘，即插件文件夹下的config.json。
	*/
	void Plugin::savePluginConfig() {
		const QString configPath = d->PluginFolder.filePath("config.json");
		Visindigo::Utility::FileOperation::ErrorCode saveResult = Visindigo::Utility::FileOperation::saveAll(configPath, d->Config.toString());
		if (saveResult != Visindigo::Utility::FileOperation::Success) {
			vgErrorF << "Failed to save config for plugin " << d->PluginID << ": " << configPath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(saveResult);
		}
	}

	/*!
		\since Visindigo 0.13.0
		从磁盘重新加载插件的配置。

		如果您的插件实现了getConfigWidget()，允许用UI配置可视化文件，那么就强烈建议您的
		配置页面在保存之后调用此函数以确保加载到内存中的设置与磁盘上的一致，
		避免出现配置页面显示的设置与实际设置不一致的情况。
	*/
	void Plugin::reloadPluginConfig() {
		d->loadConfig();
	}

	/*!
		\since Visindigo 0.16.0
		\a key 键名。

		一个便捷函数，用于获取配置中的一个布尔量。如果该键不存在或无法转换为布尔值，则返回false。

		return 指定键的值。
	*/
	bool Plugin::testPluginConfig(const QString& key) const {
		return d->Config.getBool(key);
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的加载方式。
	*/
	Plugin::LoadType Plugin::getPluginLoadType() const {
		return d->LoadType;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件包含的模块列表。
	*/
	QList<PluginModule*> Plugin::getModules() const {
		return d->Modules;
	}

	/*!
		\since Visindigo 0.13.0
		\a id 模块的ID

		return 插件中ID为 \a id 的模块，如果没有找到，则返回nullptr。

		请注意，出于从ID搜索模块可能并不常用考虑，在注册模块到插件时，
		并不会立即将它们存入Map中以节省空间和提高性能，因此使用此
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

		return 插件中全部类型ID为 \a typeID 的模块列表，如果没有找到，则返回一个空列表。

		请注意，出于从类型ID搜索模块可能并不常用考虑，在注册模块到插件时，
		并不会立即将它们存入Map中以节省空间和提高性能，因此使用此
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
		return 插件自身的版本号。
	*/
	Visindigo::General::Version Plugin::getPluginVersion() const {
		return d->PluginVersion;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件编译时所针对的Visindigo ABI版本号。
	*/
	Visindigo::General::Version Plugin::getPluginABIVersion() const {
		return d->ABIVersion;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件编译时所针对的Visindigo API版本号。
	*/
	Visindigo::General::Version Plugin::getPluginAPIVersion() const {
		return d->APIVersion;
	}
	/*!
		\since Visindigo 0.13.0
		return 插件的扩展ID。
	*/
	QString Plugin::getPluginExtensionID() const {
		return d->PluginExtensionID;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的日志记录器，其命名空间初始为插件ID，并在setPluginName()被调用后被设置为国际化后的插件名称。
	*/
	Visindigo::General::Logger* Plugin::getLogger() const {
		return d->Logger;
	}

	/*!
		\since Visindigo 0.17.0
		\a perm 被查询的权限。
		return 本插件当前是否拥有该权限。

		该函数是PluginPermissionManager::hasPermission的转发，且默认在未决定权限时
		向用户发起询问。
	*/
	bool Plugin::hasPermission(Permission perm) {
		return PluginPermissionManager::getInstance()->hasPermission(this, perm, true);
	}

	/*!
		\since Visindigo 0.17.0
		return 本插件声明的全部所需权限。

		只有在这里声明过的权限才会被PluginPermissionManager检查，
		因此插件应当在构造函数（或更早）通过setRequiredPermissions完整声明自己所需的权限，
		以便用户在授权时能掌握插件的全部敏感行为。
	*/
	Plugin::Permissions Plugin::getRequiredPermissions() {
		return d->RequiredPermissions;
	}

	/*!
		\since Visindigo 0.17.0
		\a perms 本插件所需的全部权限。

		声明插件所需的全部权限。插件应当在启用前调用此函数，
		对于未在此声明的权限，hasPermission一律返回false。
	*/
	void Plugin::setRequiredPermissions(Permissions perms) {
		d->RequiredPermissions = perms;
	}

	/*!
		\since Visindigo 0.13.0
		return 插件的配置界面，如果插件没有提供配置界面，则默认返回nullptr。

		关于此虚函数建议如下原则实现：

		懒初始化您的插件配置页面，即在第一次有人调用此函数前，
		不要创建您的插件配置页面，这有助于节省内存和提高性能。
		与此同时，建议持久化存储该指针并在禁用时再销毁它。
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
		\a name 插件的名称

		设置插件的名称。你可以按VI18N的格式给定名称的引用名称，
		这样在调用getPluginNameI18N()函数时就会返回国际化后的名称。

		设置名称的同时，插件专用Logger的命名空间也会被设置为国际化后的名称。

		有关VI18N格式的详细信息，请参阅Visindigo::General::Translator类的文档说明。
	*/
	void Plugin::setPluginName(const QString& name) {
		d->PluginName = name;
		d->Logger->setNamespace(VI18N(name));
	}
	/*!
		\since Visindigo 0.13.0
		\a author 插件的作者

		设置插件的作者。
	*/
	void Plugin::setPluginAuthor(const QStringList& author) {
		d->PluginAuthor = author;
	}
	/*!
		\since Visindigo 0.13.0
		\a description 插件的描述信息

		设置插件的描述信息。你可以按VI18N的格式给定描述信息的引用名称，
		这样在调用getPluginDescriptionI18N()函数时就会返回国际化后的描述信息。

		有关VI18N格式的详细信息，请参阅Visindigo::General::Translator类的文档说明。
	*/
	void Plugin::setPluginDescription(const QString& description) {
		d->PluginDescription = description;
	}

	/*!
		\since Visindigo 0.13.0
		\a version 插件的版本号

		设置插件自身的版本号。这个版本号不参与兼容性检查，仅供显示和插件自身逻辑使用。
	*/
	void Plugin::setPluginVersion(const Visindigo::General::Version& version) {
		d->PluginVersion = version;
	}

	/*!
		\since Visindigo 0.13.0
		\a module 要注册的模块

		注册一个插件模块。注册后，模块会出现在getModules()中，并根据其类型ID被登记到
		Visindigo对应的管理器中。重复注册同一个模块不会产生任何效果。

		\note 模块对象的所有权并未转移：模块在构造时就已经把插件作为自己的父对象，
		因此它的销毁由Qt的对象树负责。
	*/
	void Plugin::registerPluginModule(PluginModule* module) {
		if (d->Modules.contains(module)) {
			return;
		}
		d->Modules.append(module);
		if (d->ModuleTypeIDMap.contains(module->getModuleTypeID())) {
			d->ModuleTypeIDMap[module->getModuleTypeID()].append(module);
		}
		if (module->getModuleTypeID() == QString(VIModuleType_Translator)) {
			auto translator = dynamic_cast<Translator*>(module);
			if (translator) {
				TranslationHost::getInstance()->registerTranslator(translator);
			}
			else {
				vgErrorF << "Could not register translator module " << module->getModuleID() << " because it is not a Translator.";
			}
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_PlaceholderProvider)) {
			auto provider = dynamic_cast<PlaceholderProvider*>(module);
			if (provider) {
				PlaceholderManager::getInstance()->registerProvider(provider);
			}
			else {
				vgErrorF << "Could not register placeholder provider module " << module->getModuleID() << " because it is not a PlaceholderProvider.";
			}
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_CommandHandler)) {
			auto handler = dynamic_cast<CommandHandler*>(module);
			if (handler) {
				CommandHost::getInstance()->registerCommand(handler);
				handler->enable();
			}
			else {
				vgErrorF << "Could not register command handler module " << module->getModuleID() << " because it is not a CommandHandler.";
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a module 要注销的模块

		注销一个插件模块。注销后，模块会从getModules()中移除，并从Visindigo对应的管理器中注销。

		\note 这个函数只注销登记关系，不会销毁模块对象。目前它只处理Translator和PlaceholderProvider
		两种模块，命令处理程序模块需要插件自行调用其disable()。
	*/
	void Plugin::unregisterPluginModule(PluginModule* module) {
		d->Modules.removeAll(module);
		d->ModuleIDMap.remove(module->getModuleID());
		if (d->ModuleTypeIDMap.contains(module->getModuleTypeID())) {
			d->ModuleTypeIDMap[module->getModuleTypeID()].removeAll(module);
		}
		if (module->getModuleTypeID() == QString(VIModuleType_Translator)) {
			auto translator = dynamic_cast<Translator*>(module);
			if (translator) {
				TranslationHost::getInstance()->unregisterTranslator(translator);
			}
			else {
				vgErrorF << "Could not unregister translator module " << module->getModuleID() << " because it is not a Translator.";
			}
		}
		else if (module->getModuleTypeID() == QString(VIModuleType_PlaceholderProvider)) {
			auto provider = dynamic_cast<PlaceholderProvider*>(module);
			if (provider) {
				PlaceholderManager::getInstance()->unregisterProvider(provider->getModuleID());
			}
			else {
				vgErrorF << "Could not unregister placeholder provider module " << module->getModuleID() << " because it is not a PlaceholderProvider.";
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a schemeFilePath 颜色方案文件路径

		注册一个颜色方案。文件内容会被读取并交给Visindigo::Widgets::ThemeManager处理，
		读取失败或内容不合法时会在日志中记录原因，但不会抛出异常。
	*/
	void Plugin::registerColorScheme(const QString& schemeFilePath) {
		Visindigo::Utility::FileOperation::Errorable<QString> schemeResult = Visindigo::Utility::FileOperation::readAll(schemeFilePath);
		if (not schemeResult) {
			vgErrorF << "Failed to read color scheme: " << schemeFilePath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(schemeResult.error());
			return;
		}
		VISTM->pluginRegisterColorScheme(this, schemeResult.value());
	}

	/*!
		\since Visindigo 0.13.0
		\a templateFilePath 样式模板文件路径

		注册一个样式模板。文件内容会被读取并交给Visindigo::Widgets::ThemeManager处理，
		读取失败或内容不合法时会在日志中记录原因，但不会抛出异常。
	*/
	void Plugin::registerStyleTemplate(const QString& templateFilePath) {
		Visindigo::Utility::FileOperation::Errorable<QString> templateResult = Visindigo::Utility::FileOperation::readAll(templateFilePath);
		if (not templateResult) {
			vgErrorF << "Failed to read style template: " << templateFilePath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(templateResult.error());
			return;
		}
		VISTM->pluginRegisterStyleTemplate(this, templateResult.value());
	}
}

/*!
	\typedef __VisindigoPluginMain
	\relates Visindigo::General::Plugin
	\since Visindigo 0.13.0

	插件入口函数的函数指针类型，即Plugin* (*)(void)。从磁盘加载的插件必须导出符号名
	为Visindigo_PluginMain_Function_Name的函数供Visindigo解析。
*/

/*!
	\macro Visindigo_PluginMain_Function_Name
	\since Visindigo 0.13.0
	\relates Visindigo::General::Plugin

	插件入口函数的名字，值为"VisindigoPluginMain"。从磁盘加载的插件必须导出该名字的extern "C"函数。
*/

/*!
	\macro VIPlugin(PluginClass)
	\since Visindigo 0.13.0
	\relates Visindigo::General::Plugin

	在 \a PluginClass 中声明其单例访问函数。使用它之后，还需要在cpp文件中提供对应的实现。

	\badcode
		class MyPlugin : public Visindigo::General::Plugin {
			VIPlugin(MyPlugin)
		public:
			MyPlugin();
		};
	\endcode
*/

/*!
	\macro VIPluginInstance(PluginClass)
	\since Visindigo 0.13.0
	\relates Visindigo::General::Plugin

	等同于 \a PluginClass 的 \c{getInstance()}，用于快速访问该插件类的单例对象。
*/